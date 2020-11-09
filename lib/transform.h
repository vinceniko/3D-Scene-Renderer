#pragma once

#include "program.h"
#include "definitions.h"

class GLTransform {
public:
  ProgramCtx& programs_;
  std::string name_;

  GLTransform(ProgramCtx& programs, const std::string& name, const glm::mat4& mat) : GLTransform(programs, name) {
    buffer(mat);
  }
  GLTransform(ProgramCtx& programs, const std::string& name) : programs_(programs), name_(name) {}

  void buffer(const glm::mat4& mat) const {
    GLint id = programs_.get_selected_program().uniform(name_);
    if (id < 0) {
        throw std::runtime_error("Error Getting ID of Uniform");
    }

    glUniformMatrix4fv(id, 1, GL_FALSE, (float*)&mat);
    #ifdef DEBUG
    check_gl_error();
    #endif
  }
};