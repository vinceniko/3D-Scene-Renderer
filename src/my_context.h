#pragma once

#include <array>
#include <memory>

#include "context.h"

class MyContext : public Context {
    ShaderUseList shaders = { ShaderPrograms::PHONG, ShaderPrograms::FLAT, ShaderPrograms::REFLECT, ShaderPrograms::REFRACT };
    DrawModeUseList draw_modes = { DrawMode::DEF_DRAW_MODE, DrawMode::WIREFRAME, DrawMode::WIREFRAME_ONLY, DrawMode::DRAW_NORMALS  };

    std::array<Camera*, 2> cameras;
    size_t camera_idx = 0;

public:
    MyContext(int width, int height);

    // cycles through the available shader programs enumerated in ProgramList
    void switch_shader();
    void switch_draw_mode();
    void set_camera(Camera* new_camera);
    void switch_camera();
};