#pragma once

#include <functional>

#include "shader.h"
#include "definitions.h"

class GLTransform {
  std::reference_wrapper<ShaderProgramCtx> programs_;
  std::string name_;

public:
  GLTransform(ShaderProgramCtx& programs, const std::string& name, const glm::mat4& mat);
  GLTransform(ShaderProgramCtx& programs, const std::string& name);

  void buffer(const glm::mat4& mat) const;
};