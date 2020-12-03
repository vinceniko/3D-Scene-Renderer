#pragma once

#include <functional>

#include "shader.h"
#include "definitions.h"

class GLTransform {
  std::string name_;

public:
  GLTransform(const std::string& name);

  void buffer(ShaderProgram& program, const glm::mat4& mat) const;
};