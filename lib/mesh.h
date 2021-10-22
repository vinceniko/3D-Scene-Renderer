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

	glm::vec3 centroid_ = glm::vec3{ 0.f };
	glm::vec3 calc_centroid() const;

	glm::vec3 scale_ = glm::vec3{ 1.f };
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
struct RenderMesh : public Mesh, public RenderObj {
    uint32_t VAO_, VBO_, EBO_;

    void init(int VAO, uint32_t VBO, uint32_t EBO);

public:
    RenderMesh(Mesh&& mesh) : Mesh(std::move(mesh)) {
        glGenVertexArrays(1, &VAO_);
        glGenBuffers(1, &VBO_);
        glGenBuffers(1, &EBO_);

        init(VAO_, VBO_, EBO_);
    }
    RenderMesh(uint32_t VAO, uint32_t VBO, uint32_t EBO, Mesh&& mesh) : VAO_(VAO), VBO_(VBO), EBO_(EBO), Mesh(std::move(mesh)) {
        init(VAO_, VBO_, EBO_);
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
    ~RenderMesh() {
#ifdef DEBUG
        std::cout << "Deallocating RenderMesh Mem" << std::endl;
#endif
        glDeleteVertexArrays(1, &VAO_);
        glDeleteBuffers(1, &VBO_);
        glDeleteBuffers(1, &EBO_);
    }
};

class MeshFactory;

// a reference to a Mesh inside the RenderMeshFactory; represents one model being drawn;
class MeshEntity : public Spatial, public ShaderObject {
    std::reference_wrapper<MeshFactory> ctx_;

    // the index into the list of meshes in RenderMeshCtx
    size_t id_;

    Uniform u_model_trans{ "u_model_trans" };
    Uniform u_object_color{ "u_object_color" };
    glm::vec3 color_;

    MeshEntity(MeshFactory& ctx, size_t id);

public:
    friend class MeshFactory;

    const size_t get_id() const;
    void set_color(glm::vec3 new_color);
    glm::vec3 get_color() const;

    void buffer();

    void draw() override;
    // draw only with the vbo, no render state mutate, model trans buffered
    void draw_minimal();
    // draw only with the vbo, no render state mutate, model trans not buffered
    void draw_none();
    void draw_wireframe();

    float intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir) const;

    // translate, scale, and rotate back to origin, fitting into a unit cube
    void set_to_origin();
    glm::vec3 get_origin();
    glm::vec3 get_position() override;

    const RenderMesh& get_mesh();

    // TODO draw with u_model_trans_ and update u_model_trans_ on GL side
};

// a list of MeshEntity, i.e. references to Meshes inside the RenderMeshFactory; whats actually drawn
class MeshEntityList : public std::vector<std::shared_ptr<MeshEntity>> {
public:
    using std::vector<std::shared_ptr<MeshEntity>>::vector;

    MeshEntityList(const MeshEntityList&) = default;
    MeshEntityList(MeshEntityList&&) = default;
    void erase_owned(std::vector<std::shared_ptr<MeshEntity>>::const_iterator it) {
        if ((*it).use_count() == 1) {
            std::vector<std::shared_ptr<MeshEntity>>::erase(it);
            return;
        }
#ifdef DEBUG
        std::cout << "Attempting to delete unowned MeshEntity" << std::endl;
#endif
    }
    void draw();
    void draw_wireframes();
};

// holds mesh prototypes from which to generate MeshEntities
// i.e. each RenderMesh in meshes_ is unique
// all meshes used by the program get loaded in here once and may be drawn multiple times depending on how many MeshEntities are created
class MeshFactory {
    // store meshes as unique pointers to avoid copy operations and so that mem gets deallocated at the end of the program
    std::vector<std::unique_ptr<RenderMesh>> meshes_;

    // n is DefMeshList or user defined MeshList
    static size_t get_from_kind(int n) {
        return n + static_cast<int>(DefMeshList::NUM_DEF_MESHES);
    }
    static int get_from_idx(size_t n) {
        return static_cast<int>(n) - static_cast<int>(DefMeshList::NUM_DEF_MESHES);
    }

public:
    MeshFactory() {}

    MeshEntityList push(std::vector<Mesh> meshes);

    const std::vector<std::unique_ptr<RenderMesh>>& get_meshes() const;

    MeshEntity get_mesh_entity(int i);
    MeshEntity get_mesh_entity(size_t i);
};

class DefMeshFactory : public MeshFactory {
public:
    DefMeshFactory() {
        push(std::vector<Mesh>{ UnitCube{}, Quad{}, Sphere{}, Torus{} });
    }
};

extern MeshFactory* MESH_FACTORY;

void set_global_mesh_factory(MeshFactory* mesh_factory);