#pragma once

#include "glm/vec2.hpp"

#include "helpers.h"

#include "camera.h"
#include "mesh.h"

#ifdef DEBUG
#include <iostream>
#endif

class MouseContext {
    int selected_ = -1;
    bool held_ = false;

    glm::vec2 world_point_{0.f};
    glm::vec2 prev_world_point_{1.f};

    double prev_scroll_{0.f};
    double scroll_{0.f};

public:
    void hold();
    void hold(size_t i);
    void release();
    void deselect();
    int get_selected() const;
    bool is_selected() const;
    bool is_held() const;
    
    void set_world_point(glm::vec2 world_point);
    glm::vec2 get_world_point() const;
    glm::vec2 get_prev_world_point() const;
    void set_scroll(double scroll);
    double get_prev_scroll() const;
    double get_scroll() const;
};

class GLContext {
    Program& program_;

public:
    friend MeshEntity;

    GLCamera camera;
    GLMeshCtx mesh_ctx;
    MouseContext mouse_ctx;
    MeshEntityList mesh_list;

    GLContext(Program& program);
    GLContext(Program& program, GLCamera camera);

    int intersected_mesh(glm::vec3 world_ray_dir) const;
    void select(glm::vec3 world_ray_dir);
    void deselect();
    Optional<MeshEntity> get_selected();

    void init_meshes(std::vector<Mesh> meshes) {
        mesh_ctx.push(meshes);
    }
    void push_mesh_entity(std::vector<size_t> ids) {
        for (const auto& id : ids) {
            mesh_list.push_back(mesh_ctx.get_mesh_entity(id));
        }
    }

    void update();
};