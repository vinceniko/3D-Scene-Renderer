#pragma once

#include <helpers.h>

class ProgramCtx : public std::vector<Program> {
    int selected_ = -1;

public:
    void bind(int n) {
        selected_ = n;
        (*this)[selected_].bind();
    };
    Program& get_selected_program() {
        return (*this)[selected_];
    }
};