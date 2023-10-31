//
// Created by Mateusz Palkowski on 21/10/2022.
//

#include "gl_util.h"

#include <fstream>
#include <sstream>

namespace gl {
void ClearError() {
  while (glGetError() != GL_NO_ERROR)
    ;
}

bool Call(const char* function, const char* file, int line) {
  while (GLenum error = glGetError()) {
    std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file
              << ":" << line << std::endl;
    return false;
  }
  return true;
}

std::string ReadShaderSource(const std::string& file_path) {
  std::ifstream file(file_path);
  std::stringstream buffer;
  buffer << file.rdbuf();
  return {buffer.str()};
}

unsigned int CompileShader(unsigned int type, const std::string& source) {
  unsigned int id = glCreateShader(type);
  const char* src = source.c_str();
  glShaderSource(id, 1, &src, nullptr);
  glCompileShader(id);

  int result;
  glGetShaderiv(id, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

    char* message = new char[length];
    glGetShaderInfoLog(id, length, &length, message);
    std::cout << "Failed to compile shader!" << std::endl;
    std::cout << message << std::endl;
    delete[] message;
    return 0;
  }
  return id;
}

unsigned int CreateShader(const std::string& vertex_source,
                          const std::string& fragment_source) {
  unsigned int program = glCreateProgram();
  std::string vert_contents = ReadShaderSource(vertex_source);
  std::string frag_contents = ReadShaderSource(fragment_source);

  unsigned int vs = CompileShader(GL_VERTEX_SHADER, vert_contents);
  unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, frag_contents);

  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);

  glDeleteShader(vs);
  glDeleteShader(fs);

  return program;
}

}  // namespace gl