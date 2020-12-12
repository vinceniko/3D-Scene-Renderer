#pragma once

#include <fstream>
#include <vector>
#include <array>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include <iostream>

#include "definitions.h"
#include "renderer.h"
#include "rendereable.h"
#include "spatial.h"
#include "triangle.h"
#include "shader.h"

#include <optional>
#include <memory>
#include <functional>

enum DefMeshList {
    NUM_DEF_MESHES = 4,
    CUBE = -NUM_DEF_MESHES,
    QUAD,
    SPHERE,
    TORUS
};

const std::string DEF_MESH_DIR = "../data/";

// indexes into a primitive of a mesh
using Indexer = std::array<uint32_t, TRI>;

class MeshEntity;

// vertex and mesh computations
class Mesh {
    std::vector<glm::vec3> verts_;
    std::vector<Indexer> faces_;

    std::vector<glm::vec3> normals_;
    std::vector<glm::vec3> calc_normals() const;

    glm::vec3 centroid_;
    glm::vec3 calc_centroid() const;

    glm::vec3 scale_;
    glm::vec3 calc_scale() const;

protected:
    void init();

public:
    Mesh(std::vector<glm::vec3> verts, std::vector<Indexer> faces) : verts_(verts), faces_(faces) {}
    // file path constructor
    Mesh(std::string f_path);

    // accessors

    void push_back(glm::vec3 vert, const Indexer& indexer);
    const std::vector<glm::vec3>& get_verts() const;
    const std::vector<glm::vec3>& get_normals() const;
    const std::vector<Indexer>& get_faces() const;
    const glm::vec3& get_centroid() const;
    const glm::vec3& get_scale() const;

    void print() const;

    // operations
    // TODO
    void gen_normals();
};

class UnitCube : public Mesh {
public:
    UnitCube() : Mesh(DEF_MESH_DIR + "cube.off") {}
};

class Quad : public Mesh {
public:
    Quad() : Mesh(DEF_MESH_DIR + "quad.off") {}
};

class Sphere : public Mesh {
public:
    Sphere() : Mesh(DEF_MESH_DIR + "sphere.off") {}
};

class Torus : public Mesh {
public:
    Torus() : Mesh(DEF_MESH_DIR + "torus.off") {}
};

// holds mesh and GL vertex info
struct GLMesh : public Mesh {
    uint32_t VAO_, VBO_, EBO_;

    void init(ShaderProgram& program, int VAO, uint32_t VBO, uint32_t EBO);

public:
    GLMesh(ShaderProgram& program, Mesh&& mesh) : Mesh(std::move(mesh)) {
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);

        init(program, VAO_, VBO_, EBO_);
    }
    GLMesh(ShaderProgram& program, uint32_t VAO, uint32_t VBO, uint32_t EBO, Mesh&& mesh) : VAO_(VAO), VBO_(VBO), EBO_(EBO), Mesh(std::move(mesh)) {
        init(program, VAO_, VBO_, EBO_);
    }

    uint32_t get_VAO() const {
        return VAO_;
    }
    uint32_t get_VBO() const {
        return VBO_;
    }
    uint32_t get_EBO() const {
        return EBO_;
    }
    // // TODO, cannot use emplace due to vector resizing deleting mem, put freeing into ctx
    ~GLMesh() {
#ifdef DEBUG
        std::cout << "Deallocating GLMesh Mem" << std::endl;
#endif
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
    }
};

class MeshFactory;

// a reference to a Mesh inside the GLMeshFactory; represents one model being drawn;
class MeshEntity : public Spatial, public ShaderObject {
    std::reference_wrapper<MeshFactory> ctx_;

    // the index into the list of meshes in GLMeshCtx
    size_t id_;

    Uniform model_uniform_{ "u_model_trans" };
    glm::vec3 color_;

    MeshEntity(MeshFactory& ctx, size_t id);

public:
    friend class MeshFactory;

    const size_t get_id() const;
    void set_color(glm::vec3 new_color);
    glm::vec3 get_color() const;

    void buffer(ShaderProgram& program);

    void draw(ShaderProgram& program) override;
    // TODO: change to remove
    void draw_no_color(ShaderProgram& program);
    void draw_wireframe(ShaderProgram& program);

    float intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir) const;

    // translate, scale, and rotate back to origin, fitting into a unit cube
    void set_to_origin();
    glm::vec3 get_origin() {
        return glm::vec3(trans_ * glm::vec4(get_mesh().get_centroid(), 1.f));
    }
    glm::vec3 get_position() override {
        return get_mesh().get_centroid();
    }

    const GLMesh& get_mesh();

    // TODO draw with u_model_trans_ and update u_model_trans_ on GL side
};

// a list of MeshEntity, i.e. references to Meshes inside the GLMeshFactory; whats actually drawn
class MeshEntityList : public std::vector<MeshEntity> {
public:
    using std::vector<MeshEntity>::vector;

    void draw(ShaderProgram& program);
    void draw_wireframes(ShaderProgram& program);
};

// holds mesh prototypes from which to generate MeshEntities
// i.e. each GLMesh in meshes_ is unique
// all meshes used by the program get loaded in here once and may be drawn multiple times depending on how many MeshEntities are created
class MeshFactory {
    // store meshes as unique pointers to avoid copy operations and so that mem gets deallocated at the end of the program
    std::vector<std::unique_ptr<GLMesh>> meshes_;

    // n is DefMeshList or user defined MeshList
    static size_t get_from_kind(int n) {
        return n + static_cast<int>(DefMeshList::NUM_DEF_MESHES);
    }
    static int get_from_idx(size_t n) {
        return static_cast<int>(n) - static_cast<int>(DefMeshList::NUM_DEF_MESHES);
    }

    MeshFactory(ShaderProgram& program) : meshes_() {
        push(program, std::vector<Mesh>{ UnitCube{}, Quad{}, Sphere{}, Torus{} });
    }

public:
    static MeshFactory& get() {
        static MeshFactory mesh_factory{ ShaderProgramCtx::get().get_selected_program() };
        return mesh_factory;
    }

    MeshFactory(const MeshFactory&) = delete;

    MeshEntityList push(ShaderProgram& program, std::vector<Mesh> meshes);

    const std::vector<std::unique_ptr<GLMesh>>& get_meshes() const;
    
    MeshEntity get_mesh_entity(int i);
    MeshEntity get_mesh_entity(size_t i);
};
