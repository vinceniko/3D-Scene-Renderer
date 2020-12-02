#pragma once

#include <iostream>
#include <limits>
#include <exception>

#include "helpers.h"
#include "filewatcher.h"
#include "utilities.h"

const std::string SHADER_PATH = "../shaders/";

// default available programs. enumerations use negative values so that user extensions can be 0 based
// these enumerations represent indices
enum ShaderPrograms {
    NUM_SHADERS = 7,

    DEF = -ShaderPrograms::NUM_SHADERS,
    FLAT,
    PHONG,
    NORMALS,
    ENV,
    REFLECT,
    REFRACT,
};

// binds shader programs
class ShaderProgramCtx : public std::vector<ShaderProgramFile*> {
    int selected_ = -1;

    int get_selected_idx() const;
    void set_selected_idx(int n);

    FileWatcher file_watcher_{ 1000 };

public:
    ShaderProgramCtx();
    ~ShaderProgramCtx() {
        for (auto ptr: *this) {
            delete ptr;
        }
    }
    static size_t get(int n);
    void bind(ShaderPrograms n);
    ShaderProgram& get_selected_program();
    ShaderPrograms get_selected();
    void reload();
};