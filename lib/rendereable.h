#pragma once

#include "renderer.h"
#include "shader.h"

class ShaderObject {
    ShaderPrograms shader_ = ShaderPrograms::PHONG;
    DrawMode draw_mode_ = DrawMode::DEF_DRAW_MODE;
    bool dynamic_refl_ = true;

public:
    DrawMode get_draw_mode() {
        return draw_mode_;
    }
    ShaderPrograms get_shader() {
        return shader_;
    }
    virtual void set_draw_mode(DrawMode draw_mode) {
        draw_mode_ = draw_mode;
    }
    virtual void set_shader(ShaderPrograms shader) {
        shader_ = shader;
    }
    virtual void set_dyn_reflections(bool state) {
        dynamic_refl_ = state;
    }
    bool get_dyn_reflections() {
        return dynamic_refl_;
    }

    virtual void draw(ShaderProgram& program) = 0;
};