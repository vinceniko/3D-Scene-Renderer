#pragma once

#include <helpers.h>

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

#include <optional>
#include <functional>

using Indexer = std::array<uint, TRI>;

class MeshEntity;

using MeshEntityList = std::vector<MeshEntity>;

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
    const std::vector<glm::vec3>& get_verts() const {
        return verts_;
    }
    const std::vector<glm::vec3>& get_normals() const {
        return normals_;
    }
    const std::vector<Indexer>& get_faces() const {
        return faces_;
    }
    const glm::vec3& get_centroid() const {
        return centroid_;
    }
    const glm::vec3& get_scale() const {
        return scale_;
    }

    // * DEBUG
    #ifdef DEBUG
    void print() const ;
    #endif

    // operations
    // TODO
    void gen_normals();
};

class BumpyCubeMesh : public Mesh {
public:
    BumpyCubeMesh() : Mesh("../data/bumpy_cube.off") {}
};

class BunnyMesh : public Mesh {
public:
    BunnyMesh() : Mesh("../data/bunny.off") {}
};

class UnitCube : public Mesh {
public:
    UnitCube() : Mesh("../data/cube.off") {}
};

struct GLMesh : public Mesh {
    uint VAO_, VBO_, EBO_;

public:
    GLMesh(uint VAO, uint VBO, uint EBO, Mesh mesh) : VAO_(VAO), VBO_(VBO), EBO_(EBO), Mesh(mesh) {}
    // // TODO, cannot use emplace due to vector resizing deleting mem, put freeing into ctx
    // ~GLMesh() {
    //     glDeleteVertexArrays(1, &VAO_);
    //     glDeleteBuffers(1, &VBO_);
    //     glDeleteBuffers(1, &EBO_);
    // }
};

class GLMeshCtx;

// a reference to a Mesh inside the GLMeshCtx
// represents one entity being drawn
class MeshEntity {
    const GLMeshCtx& ctx_;

    // the index into the list of meshes in GLMeshCtx
    const size_t id_;

    glm::mat4 model_trans_{1.f};
    GLTransform model_uniform_;
    glm::vec3 color_;

    MeshEntity(GLMeshCtx& ctx, size_t id);
public:
    friend class GLMeshCtx;

    enum ScaleDir {
        In,
        Out,
    };

    const size_t get_id() const;
    void set_color(glm::vec3 new_color) {
        color_ = new_color;
    }
    glm::vec3 get_color() const {
        return color_;
    }

    void translate(glm::mat4 view_trans, glm::vec3 offset);
    void scale(glm::mat4 view_trans, ScaleDir dir, float offset);
    void rotate(glm::mat4 view_trans, float degrees, glm::vec3 axis);

    void draw() const;

    float intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir) const;

    void set_to_origin();

    // TODO draw with model_trans_ and update model_trans_ on GL side
};

// holds mesh prototypes
// i.e. each GLMesh in meshes_ is unique
class GLMeshCtx {
    Program& program_;

    std::vector<GLMesh> meshes_;

public:
    friend class MeshEntity;

    GLMeshCtx(Program& program) : program_(program), meshes_() {}
    
    MeshEntityList push(std::vector<Mesh> meshes);

    const std::vector<GLMesh>& get_meshes() const;
    MeshEntity get_mesh_entity(size_t i);
};
