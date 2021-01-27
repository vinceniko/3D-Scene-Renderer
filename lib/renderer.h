#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <vector>
#include <glm/glm.hpp>  // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4


#ifdef _WIN32
#  include <windows.h>
#  undef max
#  undef min
#  undef DrawText
#endif

#ifndef __APPLE__
#  define GLEW_STATIC
#  include <GL/glew.h>
#endif

#ifdef __APPLE__
#   include <OpenGL/gl3.h>
#   define __gl_h_ /* Prevent inclusion of the old gl.h */
#else
#   ifdef _WIN32
#       include <windows.h>
#   endif
#   include <GL/gl.h>
#endif

// From: https://blog.nobel-joergensen.com/2013/01/29/debugging-opengl-using-glgeterror/
bool _check_gl_error(const char* file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#endif

#include "definitions.h"
#include "utilities.h"
#include "filewatcher.h"
#include <exception>

enum ShaderType {
    FRAG,
    GEOM,
    VERT
};

// This class wraps an OpenGL program
class ShaderProgram
{
public:
    uint32_t vertex_shader;
    uint32_t fragment_shader;
    uint32_t geometry_shader;
    uint32_t program_shader;

    ShaderProgram() : vertex_shader(0), fragment_shader(0), geometry_shader(0), program_shader(0) { }
    ~ShaderProgram() { free(); }

    // Create a new shader from the specified source strings
    bool init(const std::string& vertex_shader_string,
        const std::string& geometry_shader_string,
        const std::string& fragment_shader_string,
        const std::string& fragment_data_name);

    // Select this shader for subsequent draw calls
    void bind();

    void free_vert();
    void free_geom();
    void free_frag();

    // Release all OpenGL objects
    void free();

    // Return the OpenGL handle of a named shader attribute (-1 if it does not exist)
    int32_t attrib(const std::string& name) const;

    // Return the OpenGL handle of a uniform attribute (-1 if it does not exist)
    int32_t uniform(const std::string& name) const;

    uint32_t create_shader_helper(int32_t type, const std::string& shader_string);
};

// represents Shader source code saved on the filesystem
class ShaderProgramFile : public ShaderProgram {
    std::string vert_path_;
    std::string geom_path_;
    std::string frag_path_;

public:
    ShaderProgramFile(const std::string& vertex_path,
        const Optional<std::string> geometry_path,
        const std::string& fragment_path,
        const std::string& fragment_data_name);
    ShaderProgramFile(const std::string& vertex_path,
        const Optional<std::string> geometry_path,
        const std::string& fragment_path,
        const std::string& fragment_data_name,
        FileWatcher& file_watcher);


    // Create a new shader from the specified file paths
    bool init(const std::string& vertex_path,
        const Optional<std::string> geometry_path,
        const std::string& fragment_path,
        const std::string& fragment_data_name);

    const std::string& get_vert_path() {
        return vert_path_;
    }
    const std::string& get_geom_path() {
        return geom_path_;
    }
    const std::string& get_frag_path() {
        return frag_path_;
    }

    void attach_link(uint32_t shader_id);

    // reload vert shader
    void reload_vert();
    // reload vert shader from file
    void reload_vert(const std::string& f_path);
    // reload geom shader
    void reload_geom();
    // reload geom shader from file
    void reload_geom(const std::string& f_path);
    // reload frag shader
    void reload_frag();
    // reload frag shader from file
    void reload_frag(const std::string& f_path);

    bool has_geom() {
        return geometry_shader > 0;
    }
};

class RenderObject {
public:
    virtual void init() = 0;
    virtual void free() = 0;
    virtual void bind() = 0;
};

struct Uniform {
    std::string name_;

    Uniform() {}
    Uniform(std::string name) : name_(name) {}
    void buffer(ShaderProgram& program, const float& val) {
        int32_t id = program.uniform(name_);
        check_error(id);

        glUniform1f(id, val);
#ifdef DEBUG
        check_gl_error();
#endif
    }
    void buffer(ShaderProgram& program, const glm::mat4& val) {
        int32_t id = program.uniform(name_);
        check_error(id);

        glUniformMatrix4fv(id, 1, GL_FALSE, (float*)&val[0][0]);
#ifdef DEBUG
        check_gl_error();
#endif
    }
    void buffer(ShaderProgram& program, const glm::vec3& val) {
        int32_t id = program.uniform(name_);
        check_error(id);

        glUniform3f(id, val[0], val[1], val[2]);
#ifdef DEBUG
        check_gl_error();
#endif
    }
    void buffer(ShaderProgram& program, const bool& val) {
        int32_t id = program.uniform(name_);
        check_error(id);

        glUniform1ui(id, static_cast<unsigned int>(val));
#ifdef DEBUG
        check_gl_error();
#endif
    }
    virtual void buffer(ShaderProgram& program, const int& val) {
        int32_t id = program.uniform(name_);
        check_error(id);

        glUniform1i(id, val);
#ifdef DEBUG
        check_gl_error();
#endif
    }

private:
    void check_error(int32_t id) {
        if (id < 0) {
            throw std::runtime_error("Error Getting ID of Uniform: " + name_);
        }
    }
};

class Canvas {
protected:
    int width_ = 0;
    int height_ = 0;
public:
    Canvas() = default;
    Canvas(int width, int height) : width_(width), height_(height) {}
    virtual int get_width() {
        return width_;
    }
    virtual int get_height() {
        return height_;
    }
    virtual void set_width(int width) {
        width_ = width;
    }
    virtual void set_height(int height) {
        height_ = height;
    }
    virtual void resize(int width, int height) {
        set_width(width);
        set_height(height);
    }

    virtual void reset_viewport() {
        glViewport(0, 0, width_, height_);
    }
};