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

#include <iostream>

#include "definitions.h"
#include "transform.h"

#define TRI 3
using Indexer = std::array<uint, TRI>;

class MeshEntity;

using MeshEntityList = std::vector<MeshEntity>;

class Mesh {
    std::vector<glm::vec3> verts_;
    std::vector<Indexer> faces_;

public:
    Mesh(std::vector<glm::vec3> verts, std::vector<Indexer> faces) : verts_(verts), faces_(faces) {}
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
    ) {}
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
    GLMeshCtx& ctx_;

    // the index into the list of meshes in GLMeshCtx
    const size_t id_;

    glm::mat4 model_trans_{1.f};
    GLTransform model_uniform_;
    glm::vec3 color_;

    MeshEntity(GLMeshCtx& ctx, size_t id, const GLMesh& mesh);
public:
    friend class GLMeshCtx;

    const size_t get_id() const;
    void set_color(glm::vec3 new_color) {
        color_ = new_color;
    }
    glm::vec3 get_color() {
        return color_;
    }

    void draw();

    bool intersected_triangles(glm::vec3 world_ray_origin, glm::vec3 world_ray_dir);

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
    MeshEntity push(Mesh mesh);

    const std::vector<GLMesh>& get_meshes() const;
};
