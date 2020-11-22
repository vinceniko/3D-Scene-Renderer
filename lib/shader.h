#pragma once

// #ifdef DEBUG
#include <iostream>
// #endif
#include <limits>

#include "helpers.h"

const std::string SHADER_PATH = "../shaders/";

// default available programs. enumerations use negative values so that user extensions can be 0 based
// these enumerations represent indices
enum ShaderPrograms {
    NUM_SHADERS = 4,

    DEF = -ShaderPrograms::NUM_SHADERS,
    FLAT,
    PHONG,
    NORMALS,
};

// binds shader programs
class ShaderProgramCtx : public std::vector<ShaderProgram> {
    int selected_ = -1;

    int get_selected_idx() const;
    void set_selected_idx(int n);
public:
    ShaderProgramCtx();
    void bind(ShaderPrograms n);
    ShaderProgram& get_selected_program();
    ShaderPrograms get_selected();
};