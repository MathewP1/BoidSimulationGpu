#ifndef GL_UTIL_H
#define GL_UTIL_H

#include <csignal>
#include <iostream>

#include "GL/glew.h"

#define ASSERT(x) \
  if (!(x)) std::raise(SIGINT)
#define GL_CALL(x)  \
  gl::ClearError(); \
  x;                \
  ASSERT(gl::Call(#x, __FILE__, __LINE__))

namespace gl {
void ClearError();

bool Call(const char *function, const char *file, int line);

unsigned int CreateShader(const std::string& vertex_source, const std::string& fragment_source);
}  // namespace gl

#endif