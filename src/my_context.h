#pragma once

#include <array>
#include <memory>
#include <chrono>

#include "context.h"

class MyContext : public Context {
    ShaderCycler shaders = { ShaderPrograms::PHONG, ShaderPrograms::FLAT, ShaderPrograms::REFLECT, ShaderPrograms::REFRACT };
    DrawModeCycler draw_modes = { DrawMode::DEF_DRAW_MODE, DrawMode::WIREFRAME, DrawMode::WIREFRAME_ONLY, DrawMode::DRAW_NORMALS  };

    std::array<Camera*, 2> cameras;
    size_t camera_idx = 0;

    // TODO: make list, currently supports two CubeMaps, other set in constructor
    std::unique_ptr<GL_CubeMapEntity> curr_cube_map = std::make_unique<Def_GL_CubeMapEntity>();


public:
    bool rotate_light = true;
    
    MyContext(int width, int height);

    // cycles through the available shader programs enumerated in ProgramList
    void switch_shader();
    void switch_draw_mode();
    void switch_cube_map();
    void set_camera(Camera* new_camera);
    void switch_camera();

    void update(std::chrono::duration<float> delta) {
        if (rotate_light) {
            env->dir_light_.set_trans(glm::rotate(env->dir_light_.get_trans(), glm::radians(45.f) * static_cast<float>(delta.count()), glm::vec3(0.f, 1.f, 0.f)));
            env->dir_light_.set_trans(glm::lookAt(env->dir_light_.get_position(), glm::vec3(0.f), glm::vec3(0.f, 0.f, 1.f)));
        }

        Context::update(delta);
    }
};