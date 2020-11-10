#include "helpers.h"

#include <iostream>
#include "utilities.h"

Program::Program(const std::string &vertex_path,
const Optional<std::string> geometry_path,
const std::string &fragment_path,
const std::string &fragment_data_name) : Program() {
  std::string vertex_shader_string = get_file_str(vertex_path);

  std::string geometry_shader_string = "";
  if (geometry_path.has_value()) {
    geometry_shader_string = get_file_str(geometry_path->get());
  }

  std::string fragment_shader_string = get_file_str(fragment_path);

  init(vertex_shader_string, geometry_shader_string, fragment_shader_string, fragment_data_name); 
}

bool Program::init(
  const std::string &vertex_shader_string,
  const std::string &geometry_shader_string,
  const std::string &fragment_shader_string,
  const std::string &fragment_data_name)
{
  using namespace std;
  vertex_shader = create_shader_helper(GL_VERTEX_SHADER, vertex_shader_string);
  fragment_shader = create_shader_helper(GL_FRAGMENT_SHADER, fragment_shader_string);
  if (geometry_shader_string.size() > 0) {
    geometry_shader = create_shader_helper(GL_GEOMETRY_SHADER, geometry_shader_string);
  }

  if (!vertex_shader || !fragment_shader || (geometry_shader_string.size() > 1 && !geometry_shader))
    return false;

  program_shader = glCreateProgram();

  glAttachShader(program_shader, vertex_shader);
  glAttachShader(program_shader, fragment_shader);
  if (geometry_shader_string.size() > 0) {
    glAttachShader(program_shader, geometry_shader);
  }

  glBindFragDataLocation(program_shader, 0, fragment_data_name.c_str());
  glLinkProgram(program_shader);

  GLint status;
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
  
  check_gl_error();
  return true;
}

void Program::bind()
{
  glUseProgram(program_shader);
  check_gl_error();
}

GLint Program::attrib(const std::string &name) const
{
  return glGetAttribLocation(program_shader, name.c_str());
}

GLint Program::uniform(const std::string &name) const
{
  return glGetUniformLocation(program_shader, name.c_str());
}

void Program::free()
{
  if (program_shader)
  {
    glDeleteProgram(program_shader);
    program_shader = 0;
  }
  if (vertex_shader)
  {
    glDeleteShader(vertex_shader);
    vertex_shader = 0;
  }
  if (fragment_shader)
  {
    glDeleteShader(fragment_shader);
    fragment_shader = 0;
  }
  check_gl_error();
}

GLuint Program::create_shader_helper(GLint type, const std::string &shader_string)
{
  using namespace std;
  if (shader_string.empty())
    return (GLuint) 0;

  GLuint id = glCreateShader(type);
  const char *shader_string_const = shader_string.c_str();
  glShaderSource(id, 1, &shader_string_const, NULL);
  glCompileShader(id);

  GLint status;
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
    return (GLuint) 0;
  }
  check_gl_error();

  return id;
}

void _check_gl_error(const char *file, int line)
{
  GLenum err (glGetError());

  while(err!=GL_NO_ERROR)
  {
    std::string error;

    switch(err)
    {
      case GL_INVALID_OPERATION:      error         = "INVALID_OPERATION";      break;
      case GL_INVALID_ENUM:           error         = "INVALID_ENUM";           break;
      case GL_INVALID_VALUE:          error         = "INVALID_VALUE";          break;
      case GL_OUT_OF_MEMORY:          error         = "OUT_OF_MEMORY";          break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
    }

    std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line << std::endl;
    err = glGetError();
  }
}

