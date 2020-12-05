#pragma once

#include <iostream>
#include <limits>
#include <exception>
#include <vector>

#include "renderer.h"
#include "filewatcher.h"
#include "utilities.h"

const std::string SHADER_PATH = "../shaders/";

// default available programs. enumerations use negative values so that user extensions can be 0 based
// these enumerations represent indices
enum ShaderPrograms {
    NUM_SHADERS = 7,

    DEF_SHADER = -ShaderPrograms::NUM_SHADERS,
    FLAT,
    PHONG,
    NORMALS,
    ENV,
    REFLECT,
    REFRACT,
};

// extra modes for drawing. mostly for debug purposes, such as wireframe.
enum DrawMode {
    DEF_DRAW_MODE,
    WIREFRAME,
    WIREFRAME_ONLY,
    DRAW_NORMALS,

    NUM_DRAWMODES = 4,
};

// binds shader programs
class ShaderProgramCtx : public std::vector<ShaderProgramFile*> {
    int selected_ = -1;

    int get_selected_idx() const;
    void set_selected_idx(int n);

    FileWatcher file_watcher_{ 1000 };

    ShaderProgramCtx();
    ~ShaderProgramCtx() {
        for (auto ptr : *this) {
            delete ptr;
        }
    }

public:
    static ShaderProgramCtx& get() {
        static ShaderProgramCtx shader_program;
        return shader_program;
    }

    ShaderProgramCtx(const ShaderProgramCtx&) = delete;

    static size_t get(int n);
    void bind(ShaderPrograms n);
    ShaderProgram& get_selected_program();
    ShaderPrograms get_selected();
    void reload();
};

// cycles through values of T in the vector
template <typename T>
class Cycler : std::vector<T> {
    size_t idx = 0;
public:
    using std::vector<T>::vector;

    T cycle() {
        return (*this)[(idx += 1) %= this->size()];
    }
};

// the draw modes that the program will use
class DrawModeUseList : public Cycler<DrawMode> { using Cycler::Cycler; };

// the shaders that the program will use
class ShaderUseList : public Cycler<ShaderPrograms> { using Cycler::Cycler; };
