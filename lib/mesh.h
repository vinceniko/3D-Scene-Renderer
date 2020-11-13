#pragma once

#include "helpers.h"

#include <fstream>
#include <vector>
#include <array>

#include <glm/vec3.hpp> // glm::vec3
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#ifdef DEBUG
#include <iostream>
#endif

#include "definitions.h"
#include "transform.h"
#include "triangle.h"
#include "shader.h"

#include <optional>
#include <functional>

// indexes into a primitive of a mesh
using Indexer = std::array<uint, TRI>;

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

    // * DEBUG
#ifdef DEBUG
    void print() const;
#endif

    // operations
    // TODO
    void gen_normals();
};

// holds mesh and GL vertex info
struct GLMesh : public Mesh {
    uint VAO_, VBO_, EBO_;

public:
    GLMesh(uint VAO, uint VBO, uint EBO, Mesh&& mesh) : VAO_(VAO), VBO_(VBO), EBO_(EBO), Mesh(mesh) {}
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
class MeshEntity {
    std::reference_wrapper<MeshFactory> ctx_;

    // the index into the list of meshes in GLMeshCtx
    size_t id_;

    glm::mat4 model_trans_{ 1.f };
    GLTransform model_uniform_;
    glm::vec3 color_;

    MeshEntity(MeshFactory& ctx, size_t id);

public:
    friend class MeshFactory;

    enum ScaleDir {
        In,
        Out,
    };

    const size_t get_id() const;
    void set_color(glm::vec3 new_color);
    glm::vec3 get_color() const;

    void translate(glm::mat4 view_trans, glm::vec3 offset);
    void scale(glm::mat4 view_trans, ScaleDir dir, float offset);
    void rotate(glm::mat4 view_trans, float degrees, glm::vec3 axis);

    void draw();
    void draw_wireframe();

    float intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir) const;

    // translate, scale, and rotate back to origin, fitting into a unit cube
    void set_to_origin();

    // TODO draw with model_trans_ and update model_trans_ on GL side
};

// a list of MeshEntity, i.e. references to Meshes inside the GLMeshFactory; whats actually drawn
class MeshEntityList : public std::vector<MeshEntity> {
public:
    void draw();
    void draw_wireframe();
};

// holds mesh prototypes from which to generate MeshEntities
// i.e. each GLMesh in meshes_ is unique
// all meshes used by the program get loaded in here once and may be drawn multiple times depending on how many MeshEntities are created
class MeshFactory {
    std::reference_wrapper<ShaderProgramCtx> programs_;

    // store meshes as unique pointers to avoid copy operations and so that mem gets deallocated at the end of the program
    std::vector<std::unique_ptr<GLMesh>> meshes_;

public:
    friend class MeshEntity;

    MeshFactory(ShaderProgramCtx& programs) : programs_(programs), meshes_() {}

    MeshEntityList push(std::vector<Mesh> meshes);

    const std::vector<std::unique_ptr<GLMesh>>& get_meshes() const;
    MeshEntity get_mesh_entity(size_t i);
};
