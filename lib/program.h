#pragma once

// #ifdef DEBUG
#include <iostream>
// #endif
#include <limits>

#include "helpers.h"

const std::string SHADER_PATH = "../shaders/";

enum ProgramList {
    SIZE = 4,

    DEF = -ProgramList::SIZE,
    FLAT,
    PHONG,
    NORMALS,
};

class ProgramCtx : public std::vector<Program> {
    int selected_ = -1;

    int get_selected_idx() {
        return selected_;
    }
    void set_selected_idx(int n) {
        selected_ = n;
    }
public:
    ProgramCtx() {
        push_back(Program{SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "def_frag.glsl", "out_color"});
        push_back(Program{SHADER_PATH + "phong_vert.glsl", {}, SHADER_PATH + "flat_frag.glsl", "out_color"});
        push_back(Program(SHADER_PATH + "phong_vert.glsl", {}, SHADER_PATH + "phong_frag.glsl", "out_color"));
        std::string normal_geom = std::string(SHADER_PATH + "normal_geom.glsl");
        push_back(Program(SHADER_PATH + "normal_vert.glsl", {normal_geom}, SHADER_PATH + "normal_frag.glsl", "out_color"));
    }
    void bind(ProgramList n) {
        // #ifdef DEBUG
        // std::cout << "n1: " << n << std::endl;
        // #endif

        selected_ = static_cast<int>(n) + static_cast<int>(ProgramList::SIZE);

        // #ifdef DEBUG
        // std::cout << "n2: " << n << std::endl;
        // #endif

        (*this)[selected_].bind();
    };
    Program& get_selected_program() {
        return (*this)[selected_];
    }
    ProgramList get_selected() {
        return static_cast<ProgramList>(selected_ - + static_cast<int>(ProgramList::SIZE));
    }
};