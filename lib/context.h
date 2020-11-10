#pragma once

#include <array>

#include "glm/vec2.hpp"

#include "helpers.h"

#include "camera.h"
#include "mesh.h"
#include "program.h"

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
    std::array<ProgramList, 2> shaders = { ProgramList::PHONG, ProgramList::FLAT };
    
    enum DrawMode {
        DEF,
        WIREFRAME,
        NORMALS,

        SIZE = 3,
    };

public:
    friend MeshEntity;

    DrawMode draw_mode = DrawMode::DEF;
    size_t shader_idx = 0;

    std::shared_ptr<ProgramCtx> programs;
    
    GLCamera camera;
    
    MouseContext mouse_ctx;

    GLMeshCtx mesh_ctx;
    MeshEntityList mesh_list;

    GLContext(std::unique_ptr<ProgramCtx> programs);
    GLContext(std::unique_ptr<ProgramCtx> programs, std::shared_ptr<Camera> camera);

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

    void switch_draw_mode() {
        draw_mode = static_cast<DrawMode>((static_cast<int>(draw_mode) + 1) % DrawMode::SIZE);
        #ifdef DEBUG
        std::cout << "draw_mode: " << draw_mode << std::endl;
        #endif
    }

    void switch_program() {
        programs->bind(shaders[(shader_idx += 1) %= shaders.size()]);
    }

    void draw() {
        programs->bind(programs->get_selected());
        draw_surface();
        if (draw_mode == DrawMode::WIREFRAME) {
            draw_wireframe();
        } else if (draw_mode == DrawMode::NORMALS) {
            draw_normals();
        }
    }
    void draw_surface() {
        update();
        mesh_list.draw();
    }
    void draw_wireframe() {
        float min_zoom = 1.f / 4096.f;  // to prevent z-fighting

        camera->zoom(Camera::ZoomDir::In, min_zoom);
        camera.buffer();
        mesh_list.draw_wireframe();
        camera->zoom(Camera::ZoomDir::In, -min_zoom);
    }
    void draw_normals() {
        ProgramList selected = programs->get_selected();
        
        programs->bind(ProgramList::NORMALS);;
        camera.buffer();

        for (auto& mesh : mesh_list) {
            auto temp = mesh.get_color();
            mesh.set_color(glm::vec3(1.0, 0.0, 0.0));
            mesh.draw();
            mesh.set_color(temp);
        }

        mesh_list.draw();

        programs->bind(selected);
    }

    void update();
};