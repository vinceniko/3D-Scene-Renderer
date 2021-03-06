#include "renderer.h"

#include <iostream>

ShaderProgramFile::ShaderProgramFile(const std::string& vertex_path,
    const Optional<std::string> geometry_path,
    const std::string& fragment_path,
    const std::string& fragment_data_name) :
    ShaderProgram(),
    vert_path_(vertex_path),
    frag_path_(fragment_path) {
    init(vertex_path, geometry_path, fragment_path, fragment_data_name);
}

ShaderProgramFile::ShaderProgramFile(const std::string& vertex_path,
    const Optional<std::string> geometry_path,
    const std::string& fragment_path,
    const std::string& fragment_data_name,
    FileWatcher& file_watcher) :
    ShaderProgramFile(vertex_path, geometry_path, fragment_path, fragment_data_name) {
    file_watcher.add_path(get_vert_path(), {});
    if (get_geom_path().size() > 0) file_watcher.add_path(get_geom_path(), {});
    file_watcher.add_path(get_frag_path(), {});
}

bool ShaderProgramFile::init(const std::string& vertex_path,
    const Optional<std::string> geometry_path,
    const std::string& fragment_path,
    const std::string& fragment_data_name) {
    std::string vertex_shader_string = get_file_str(vertex_path);

    std::string geometry_shader_string = "";
    if (geometry_path.has_value()) {
        geom_path_ = geometry_path->get();
        geometry_shader_string = get_file_str(geometry_path->get());
    }

    std::string fragment_shader_string = get_file_str(fragment_path);

    return ShaderProgram::init(vertex_shader_string, geometry_shader_string, fragment_shader_string, fragment_data_name);
}

void ShaderProgram::attach_link(uint32_t shader_id) {
    glAttachShader(program_shader, shader_id);
    glLinkProgram(program_shader);

    int32_t status;
    glGetProgramiv(program_shader, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetProgramInfoLog(program_shader, 512, NULL, buffer);
        throw std::runtime_error("Linker error");
        // std::cerr << "Linker error: " << std::endl << buffer << std::endl;
        program_shader = 0;
    }

    if (check_gl_error()) throw std::runtime_error("Error in Source");
}

void ShaderProgramFile::reload_vert() {
    free_vert();

    std::string vert_str = get_file_str(get_vert_path());
    // #ifdef DEBUG
    //     std::cout << "true vert" << std::endl;
    // #endif
    vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vert_str);

    attach_link(vertex_shader);
}

void ShaderProgramFile::reload_vert(const std::string& f_path) {
    vert_path_ = f_path;

    reload_vert();
}

void ShaderProgramFile::reload_geom() {
    free_geom();

    std::string geom_str = get_file_str(get_geom_path());
#ifdef DEBUG
    std::cout << "true geom" << std::endl;
#endif
    geometry_shader = create_shader_helper(GL_GEOMETRY_SHADER, geom_str);

    attach_link(geometry_shader);
}

void ShaderProgramFile::reload_geom(const std::string& f_path) {
    geom_path_ = f_path;

    reload_geom();
}

void ShaderProgramFile::reload_frag() {
    free_frag();

    std::string frag_str = get_file_str(get_frag_path());
    // #ifdef DEBUG
    //     std::cout << "true" << std::endl;
    // #endif
    fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, frag_str);

    attach_link(fragment_shader);
}

void ShaderProgramFile::reload_frag(const std::string& f_path) {
    frag_path_ = f_path;

    reload_frag();
}

bool ShaderProgram::init(
    const std::string& vertex_shader_string,
    const std::string& geometry_shader_string,
    const std::string& fragment_shader_string,
    const std::string& fragment_data_name)
{
    using namespace std;
    vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vertex_shader_string);
    fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, fragment_shader_string);
    if (geometry_shader_string.size() > 0) {
        geometry_shader = create_shader_helper(GL_GEOMETRY_SHADER, geometry_shader_string);
    }

    if (!vertex_shader || !fragment_shader || (geometry_shader_string.size() > 0 && !geometry_shader))
        return false;

    program_shader = glCreateProgram();

    glAttachShader(program_shader, vertex_shader);
    glAttachShader(program_shader, fragment_shader);
    if (geometry_shader_string.size() > 0) {
        glAttachShader(program_shader, geometry_shader);
    }

    glBindFragDataLocation(program_shader, 0, fragment_data_name.c_str());
    glLinkProgram(program_shader);

    int32_t status;
    glGetProgramiv(program_shader, GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        char buffer[512];
        glGetProgramInfoLog(program_shader, 512, NULL, buffer);
        cerr << "Linker error: " << endl << buffer << endl;
        program_shader = 0;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#ifdef DEBUG
    check_gl_error();
#endif

    return true;
}

void ShaderProgram::bind()
{
    glUseProgram(program_shader);
#ifdef DEBUG
    check_gl_error();
#endif
}

int32_t ShaderProgram::attrib(const std::string& name) const
{
    return glGetAttribLocation(program_shader, name.c_str());
}

int32_t ShaderProgram::uniform(const std::string& name) const
{
    return glGetUniformLocation(program_shader, name.c_str());
}

void ShaderProgram::free_vert() {
    if (vertex_shader) {
        glDetachShader(program_shader, vertex_shader);
        glDeleteShader(vertex_shader);
        vertex_shader = 0;
    }
}
void ShaderProgram::free_geom() {
    if (geometry_shader) {
        glDetachShader(program_shader, geometry_shader);
        glDeleteShader(geometry_shader);
        geometry_shader = 0;
    }
}
void ShaderProgram::free_frag() {
    if (fragment_shader) {
        glDetachShader(program_shader, fragment_shader);
        glDeleteShader(fragment_shader);
        fragment_shader = 0;
    }
}

void ShaderProgram::free()
{
    free_vert();
    free_geom();
    free_frag();
#ifdef DEBUG
    check_gl_error();
#endif
}

uint32_t ShaderProgram::create_shader_helper(int32_t type, const std::string& shader_string)
{
    using namespace std;
    if (shader_string.empty())
        return (uint32_t)0;

    uint32_t id = glCreateShader(type);
    const char* shader_string_const = shader_string.c_str();
    glShaderSource(id, 1, &shader_string_const, NULL);
    glCompileShader(id);

    int32_t status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);

    if (status != GL_TRUE)
    {
        char buffer[512];
        if (type == GL_VERTEX_SHADER)
            cerr << "Vertex shader:" << endl;
        else if (type == GL_FRAGMENT_SHADER)
            cerr << "Fragment shader:" << endl;
        else if (type == GL_GEOMETRY_SHADER)
            cerr << "Geometry shader:" << endl;
        cerr << shader_string << endl << endl;
        glGetShaderInfoLog(id, 512, NULL, buffer);
        cerr << "Error: " << endl << buffer << endl;
        return (uint32_t)0;
    }
#ifdef DEBUG
    check_gl_error();
#endif

    return id;
}

bool _check_gl_error(const char* file, int line)
{
    GLenum err(glGetError());

    bool err_ = false;
    while (err != GL_NO_ERROR)
    {
        std::string error;

        switch (err)
        {
        case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
        case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
        }

        std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
        err = glGetError();
        err_ = true;
    }
    return err_;
}

int Renderer::get_selected_idx() const {
    return selected_;
}
void Renderer::set_selected_idx(int n) {
    selected_ = n;
}

DefRenderer::DefRenderer() {
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "def_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "flat_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "phong_frag.glsl", "out_color", file_watcher_ }));
    std::string normal_geom = std::string(SHADER_PATH + "normal_geom.glsl");
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "normal_vert.glsl", { normal_geom }, SHADER_PATH + "normal_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "env_vert.glsl", {}, SHADER_PATH + "env_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "reflect_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "refract_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "shadow_vert.glsl", {}, SHADER_PATH + "shadow_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "depth_map_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "outline_vert.glsl", {}, SHADER_PATH + "def_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "def_vert.glsl", {}, SHADER_PATH + "grid_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "offscreen_vert.glsl", {}, SHADER_PATH + "offscreen_frag.glsl", "out_color", file_watcher_ }));
    push_back(std::unique_ptr<ShaderProgramFile>(new ShaderProgramFile{ SHADER_PATH + "offscreen_vert.glsl", {}, SHADER_PATH + "fxaa_frag.glsl", "out_color", file_watcher_ }));

    bind(ShaderPrograms::PHONG);
}

// global renderer
Renderer* RENDERER = nullptr;

void set_global_renderer(Renderer* renderer) {
    RENDERER = renderer;
}

size_t Renderer::get(int n) {
    return static_cast<int>(n) + static_cast<int>(ShaderPrograms::NUM_SHADERS);
}

void Renderer::bind(ShaderPrograms n) {
    // #ifdef DEBUG
    // std::cout << "n1: " << n << std::endl;
    // #endif

    // #ifdef DEBUG
    // std::cout << "n2: " << n << std::endl;
    // #endif
    selected_ = get(n);
    (*this)[selected_]->bind();
};
ShaderProgram& Renderer::get_selected_program() {
    return *(*this)[selected_];
}
ShaderProgram& Renderer::get_selected_program() const {
    return *(*this)[selected_];
}
ShaderPrograms Renderer::get_selected() {
    return static_cast<ShaderPrograms>(selected_ - +static_cast<int>(ShaderPrograms::NUM_SHADERS));
}
void Renderer::reload() {
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
        }
        catch (const std::runtime_error& e) {
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
