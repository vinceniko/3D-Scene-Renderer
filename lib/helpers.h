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
void _check_gl_error(const char* file, int line);

///
/// Usage
/// [... some opengl calls]
/// glCheckError();
///
#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

#endif

#include "definitions.h"

// This class wraps an OpenGL program composed of two shaders
class ShaderProgram
{
public:
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint geometry_shader;
    GLuint program_shader;

    ShaderProgram() : vertex_shader(0), fragment_shader(0), geometry_shader(0), program_shader(0) { }
    ShaderProgram(const std::string& vertex_path,
        const Optional<std::string> geometry_path,
        const std::string& fragment_path,
        const std::string& fragment_data_name);

    // Create a new shader from the specified source strings
    bool init(const std::string& vertex_shader_string,
        const std::string& geometry_shader_string,
        const std::string& fragment_shader_string,
        const std::string& fragment_data_name);

    // Select this shader for subsequent draw calls
    void bind();

    // Release all OpenGL objects
    void free();

    // Return the OpenGL handle of a named shader attribute (-1 if it does not exist)
    GLint attrib(const std::string& name) const;

    // Return the OpenGL handle of a uniform attribute (-1 if it does not exist)
    GLint uniform(const std::string& name) const;

    GLuint create_shader_helper(GLint type, const std::string& shader_string);
};
