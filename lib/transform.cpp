#include "transform.h"

GLTransform::GLTransform(const std::string& name) : name_(name) {}

void GLTransform::buffer(ShaderProgram& program, const glm::mat4& mat) const {
    int32_t id = program.uniform(name_);
    if (id < 0) {
        throw std::runtime_error("Error Getting ID of Uniform");
    }

    glUniformMatrix4fv(id, 1, GL_FALSE, (float*)&mat);
#ifdef DEBUG
    check_gl_error();
#endif
}