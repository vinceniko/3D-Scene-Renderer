#pragma once

#include <memory>

#include "shader.h"
#include "definitions.h"

class GLTransform {
  std::shared_ptr<ShaderProgramCtx> programs_;
  std::string name_;

public:
  GLTransform(std::shared_ptr<ShaderProgramCtx> programs, const std::string& name, const glm::mat4& mat);
  GLTransform(std::shared_ptr<ShaderProgramCtx> programs, const std::string& name);

  void buffer(const glm::mat4& mat) const;
};