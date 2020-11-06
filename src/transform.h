#pragma once

#include "helpers.h"
#include "definitions.h"

class GLTransform {
public:
  int id_;

  GLTransform(const Program& program, const std::string& name, const glm::mat4& mat) : GLTransform(program, name) {
    buffer(mat);
  }
  GLTransform(const Program& program, const std::string& name) {
    uint id = program.uniform(name);
    if (id < 0) {
        throw std::runtime_error("Error Getting ID of Uniform");
    }

    id_ = id;
  }

  void buffer(const glm::mat4& mat) {
    glUniformMatrix4fv(id_, 1, GL_FALSE, (float*)&mat);
    #ifdef DEBUG
    check_gl_error();
    #endif
  }
};