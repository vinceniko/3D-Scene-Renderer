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

    int intersected_mesh(glm::vec3 world_ray_dir) {
        for (size_t i = 0; i < mesh_list.size(); i++) {
            if (mesh_list[i].intersected_triangles(camera.get_position(), world_ray_dir)) {
                return i;
            }
        }
        return -1;
    }
    void select(glm::vec3 world_ray_dir) {
        int found = intersected_mesh(world_ray_dir);
        if (found >= 0) {
            mouse_ctx.hold(found);
        }
    }

    void update() {
        glm::vec2 old_point = mouse_ctx.get_prev_world_point();
        glm::vec2 new_point = mouse_ctx.get_world_point();

        // #ifdef DEBUG
        // std::cout << "new_point: " << new_point[0] << ' ' << new_point[1] << ' ' << 
        // "old_point: " << old_point[0] << ' ' << old_point[1] << ' ' << std::endl;
        // #endif

        glm::vec2 diff = glm::vec2(new_point.x - old_point.x, -(new_point.y - old_point.y));
        const float diff_min = 0.001;
        if (mouse_ctx.is_held()) {
            camera.translate(glm::abs(diff.x) > diff_min || glm::abs(diff.y) > diff_min ? diff : glm::vec2(0.f));
        }

        if (mouse_ctx.is_selected()) {
            std::cout << "selected: " << mouse_ctx.get_selected() << std::endl;
        }
    }
};