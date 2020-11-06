#pragma once

#include <helpers.h>

#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <exception>

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

enum MeshList {
    CUBE,
    BUMPY,
    BUNNY,
};

using Indexer = std::array<uint, TRI>;

class MeshEntity;

using MeshEntityList = std::vector<MeshEntity>;

class Mesh {
    std::vector<glm::vec3> verts_;
    std::vector<Indexer> faces_;

    glm::vec3 centroid_;
    glm::vec3 calc_centroid() const;

    glm::vec3 scale_;
    glm::vec3 calc_scale() const;

protected:
    void init() {
        centroid_ = calc_centroid();
        scale_ = calc_scale();
    }

public:
    Mesh(std::vector<glm::vec3> verts, std::vector<Indexer> faces) : verts_(verts), faces_(faces), centroid_(calc_centroid()) {}
    // file path constructor
    Mesh(std::string f_path);
    
    // accessors
    void push_back(glm::vec3 vert, Indexer indexer);
    const std::vector<glm::vec3>& get_verts() const {
        return verts_;
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
    void print() const ;

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
    UnitCube() : Mesh(
        std::vector<glm::vec3>{
            glm::vec3(0.5f,  0.5f, -0.5f),  // top right
            glm::vec3(0.5f, -0.5f, -0.5f),  // bottom right
            glm::vec3(-0.5f, -0.5f, -0.5f),  // bottom left
            glm::vec3(-0.5f,  0.5f, -0.5f),   // top left

            glm::vec3(0.5f,  0.5f, 0.5f),  // top right
            glm::vec3(0.5f, -0.5f, 0.5f),  // bottom right
            glm::vec3(-0.5f, -0.5f, 0.5f),  // bottom left
            glm::vec3(-0.5f,  0.5f, 0.5f),   // top left
        },
        std::vector<Indexer>{  // note that we start from 0!
            // front
            Indexer{0, 1, 3},   // first triangle
            Indexer{1, 2, 3},    // second triangle
        
            // back
            Indexer{4, 5, 7},
            Indexer{5, 6, 7},

            // right
            Indexer{3, 2, 7},
            Indexer{2, 6, 7},

            // left
            Indexer{4, 5, 0},
            Indexer{5, 1, 0},

            // top
            Indexer{0, 3, 4},
            Indexer{3, 7, 4},

            // bottom
            Indexer{1, 2, 5},
            Indexer{2, 6, 5},
        }
    ) {
        init();
    }
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
    glm::vec3 get_color() {
        return color_;
    }

    void translate(glm::mat4 view_trans, glm::vec3 offset);
    void scale(glm::mat4 view_trans, ScaleDir dir, float offset);
    void rotate(glm::mat4 view_trans, float degrees, glm::vec3 axis);

    void draw();

    float intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir);

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
