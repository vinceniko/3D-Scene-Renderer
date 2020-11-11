#include "transform.h"

GLTransform::GLTransform(std::shared_ptr<ShaderProgramCtx> programs, const std::string& name, const glm::mat4& mat) : GLTransform(programs, name) {
    buffer(mat);
}
GLTransform::GLTransform(std::shared_ptr<ShaderProgramCtx> programs, const std::string& name) : programs_(programs), name_(name) {}

void GLTransform::buffer(const glm::mat4& mat) const {
    GLint id = programs_->get_selected_program().uniform(name_);
    if (id < 0) {
        throw std::runtime_error("Error Getting ID of Uniform");
    }

    glUniformMatrix4fv(id, 1, GL_FALSE, (float*)&mat);
    #ifdef DEBUG
    check_gl_error();
    #endif
}