#include "shader.h"

int ShaderProgramCtx::get_selected_idx() const {
    return selected_;
}
void ShaderProgramCtx::set_selected_idx(int n) {
    selected_ = n;
}

ShaderProgramCtx::ShaderProgramCtx() {
    push_back(ShaderProgram{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "def_frag.glsl", "out_color" });
    push_back(ShaderProgram{ SHADER_PATH + "phong_vert.glsl", {}, SHADER_PATH + "flat_frag.glsl", "out_color" });
    push_back(ShaderProgram{ SHADER_PATH + "phong_vert.glsl", {}, SHADER_PATH + "phong_frag.glsl", "out_color" });
    std::string normal_geom = std::string(SHADER_PATH + "normal_geom.glsl");
    push_back(ShaderProgram{ SHADER_PATH + "normal_vert.glsl", { normal_geom }, SHADER_PATH + "normal_frag.glsl", "out_color" });
}
void ShaderProgramCtx::bind(ShaderPrograms n) {
    // #ifdef DEBUG
    // std::cout << "n1: " << n << std::endl;
    // #endif

    selected_ = static_cast<int>(n) + static_cast<int>(ShaderPrograms::SIZE);

    // #ifdef DEBUG
    // std::cout << "n2: " << n << std::endl;
    // #endif

    (*this)[selected_].bind();
};
ShaderProgram& ShaderProgramCtx::get_selected_program() {
    return (*this)[selected_];
}
ShaderPrograms ShaderProgramCtx::get_selected() {
    return static_cast<ShaderPrograms>(selected_ - +static_cast<int>(ShaderPrograms::SIZE));
}