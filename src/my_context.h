#pragma once

#include <array>
#include <memory>

#include "context.h"

class MyContext : public Context {
    // the shaders that the program will use
    std::array<ShaderPrograms, 2> shaders = { ShaderPrograms::PHONG, ShaderPrograms::FLAT };
    // an index into the list of program shaders
    size_t shader_idx = 0;

    std::array<std::shared_ptr<Camera>, 2> cameras;
    size_t camera_idx = 0;

public:
    MyContext(std::unique_ptr<ShaderProgramCtx> programs, float width, float height);

    // cycles through the available shader programs enumerated in ProgramList
    void switch_program();
    void set_camera(std::shared_ptr<Camera> new_camera);
    void switch_camera();
};