#pragma once

#include <functional>

#include "shader.h"
#include "definitions.h"

class GLTransform {
  std::string name_;

public:
  GLTransform(const std::string& name);

  void buffer(ShaderProgramCtx& programs, const glm::mat4& mat) const;
};