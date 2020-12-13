#include "shader.h"

int ShaderProgramCtx::get_selected_idx() const {
    return selected_;
}
void ShaderProgramCtx::set_selected_idx(int n) {
    selected_ = n;
}

ShaderProgramCtx::ShaderProgramCtx() {
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "def_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "flat_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "phong_frag.glsl", "out_color", file_watcher_ }));
    std::string normal_geom = std::string(SHADER_PATH + "normal_geom.glsl");
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "normal_vert.glsl", { normal_geom }, SHADER_PATH + "normal_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "env_vert.glsl", {}, SHADER_PATH + "env_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "reflect_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "refract_frag.glsl", "out_color", file_watcher_ }));
    
    bind(ShaderPrograms::DEF_SHADER);
}

size_t ShaderProgramCtx::get(int n) {
    return static_cast<int>(n) + static_cast<int>(ShaderPrograms::NUM_SHADERS);
}

void ShaderProgramCtx::bind(ShaderPrograms n) {
    // #ifdef DEBUG
    // std::cout << "n1: " << n << std::endl;
    // #endif

    // #ifdef DEBUG
    // std::cout << "n2: " << n << std::endl;
    // #endif
    selected_ = get(n);
    (*this)[selected_]->bind();
};
ShaderProgram& ShaderProgramCtx::get_selected_program() {
    return *(*this)[selected_];
}
ShaderPrograms ShaderProgramCtx::get_selected() {
    return static_cast<ShaderPrograms>(selected_ - +static_cast<int>(ShaderPrograms::NUM_SHADERS));
}
void ShaderProgramCtx::reload() {
    // reload all that were changed
    for (auto& shader : *this) {
        // TODO: currently the errored shader will be loaded and gl will print errors though the program wont crash. drawing can then be resumed by fixing the error in the source
        // a more graceful solution is to load the original program. do not free the original shader then
        // auto clone(*shader);
        ShaderType errored;
        try {
            if (file_watcher_.check_change(shader->get_vert_path())) {
                errored = ShaderType::VERT;
                shader->reload_vert();
            }
            if (shader->has_geom() && file_watcher_.check_change(shader->get_geom_path())) {
                errored = ShaderType::GEOM;
                shader->reload_geom();
            }
            if (file_watcher_.check_change(shader->get_frag_path())) {
                errored = ShaderType::FRAG;
                shader->reload_frag();
            }
        } catch (const std::runtime_error& e) {
            std::cout << e.what() << std::endl;
            // *shader = clone;

            // switch (errored) {
            // case ShaderType::VERT:
            //     shader->reload_vert();
            //     break;
            // case ShaderType::GEOM:
            //     shader->reload_geom();
            //     break;
            // case ShaderType::FRAG:
            //     shader->reload_frag();
            //     break;
            // }
        }
    }
    // mark all as unchanged
    for (auto& shader : *this) {
        file_watcher_.set_unchanged(shader->get_vert_path());
        if (shader->has_geom()) {
            file_watcher_.set_unchanged(shader->get_geom_path());
        }
        file_watcher_.set_unchanged(shader->get_frag_path());
    }
}
