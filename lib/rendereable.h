#pragma once

#include "renderer.h"
#include "shader.h"

class ShaderObject {
    ShaderPrograms shader_ = ShaderPrograms::PHONG;
    DrawMode draw_mode_ = DrawMode::DEF_DRAW_MODE;

public:
    DrawMode get_draw_mode() {
        return draw_mode_;
    }
    ShaderPrograms get_shader() {
        return shader_;
    }
    void set_draw_mode(DrawMode draw_mode) {
        draw_mode_ = draw_mode;
    }
    void set_shader(ShaderPrograms shader) {
        shader_ = shader;
    }

    virtual void draw(ShaderProgram& program) = 0;
};