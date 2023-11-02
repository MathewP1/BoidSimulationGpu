//
// Created by Mateusz Palkowski on 01/11/2023.
//

#include "simulation.h"
#include "gl_util.h"
#include "GL/glew.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <random>
#include <cmath>


namespace {
const char* kComputeVertexShader = R"(#version 410 core
in vec4 oldPosition;
in vec2 oldVelocity;

uniform float deltaTime;
uniform vec2 canvasDimensions;
uniform vec3 mousePos;

out vec4 newPosition;

vec2 wrap(vec2 pos, vec2 screen) {
  return mod(mod(pos, screen) + screen, screen);
}

void main() {
  float mouseFactor = 300.0f;
  vec2 target = mousePos.xy - oldPosition.xy;
  vec2 direction = normalize(target);
  vec2 newVelocity = oldVelocity.xy + direction * mouseFactor * mousePos.z;

  vec2 pos = oldPosition.xy + newVelocity * deltaTime;

  newPosition = vec4(mod(pos.xy, canvasDimensions), oldPosition.z, oldPosition.w);
}

)";

const char* kComputeFragmentShader = R"(#version 410 core
precision highp float;
void main(){
}
)";

const char* kVertexShader = R"(#version 410 core
in vec4 position;

uniform mat4 transform;

out float speedMagnitude;

void main() {
//mat4 orthoMatrix = mat4(
//        400.0, 0.0, 0.0, 0.0,
//        0.0, 300.0, 0.0, 0.0,
//        0.0, 0.0, -1.0, 0.0,
//        -1.0, -1.0, 0.0, 1.0
//    );


gl_Position = transform * vec4(position.xy, 0, 1);
gl_PointSize = position.z;
speedMagnitude = int(position.w);
}
)";

const char* kFragmentShader = R"(#version 410 core
precision highp float;

in float speedMagnitude;

out vec4 color;

void main() {
  float normalizedSpeed = abs(speedMagnitude) / 300.0;
// Normalize the speedMagnitude to range [0, 1]
//    float normalizedSpeed = clamp(speedMagnitude / maxSpeedMagnitude, 0.0, 1.0);

    vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
    vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
    vec4 red = vec4(1.0, 0.0, 0.0, 1.0);

    // Interpolation
    if (normalizedSpeed < 0.5) {
        // Interpolate between green and yellow for the lower half
        color = mix(green, yellow, normalizedSpeed * 2.0); // We multiply by 2 to map [0, 0.5] to [0, 1]
    } else {
        // Interpolate between yellow and red for the upper half
        color = mix(yellow, red, (normalizedSpeed - 0.5) * 2.0); // We subtract 0.5 and then multiply by 2 to map [0.5, 1] to [0, 1]
    }
}
)";

bool CheckShaderError(GLuint id) {
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
    return false;
  }
  return true;
}

bool CheckLinkError(GLuint id) {
  GLint linkStatus;
  glGetProgramiv(id, GL_LINK_STATUS, &linkStatus);
  if (linkStatus == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> infoLog(maxLength);
    glGetProgramInfoLog(id, maxLength, &maxLength, &infoLog[0]);

    std::cerr << "Program Linking Error: " << &infoLog[0] << std::endl;

    // Optionally delete the program to avoid further issues
    glDeleteProgram(id);
    return false;
  }
  return true;
}
}

bool Simulation::SetupGl() {
  gl_.compute_program = glCreateProgram();

  auto compute_shader = glCreateShader(GL_VERTEX_SHADER);
  GL_CALL(glShaderSource(compute_shader, 1, &kComputeVertexShader, nullptr));
  GL_CALL(glCompileShader(compute_shader));

  if (!CheckShaderError(compute_shader)) {
    return false;
  }

  auto compute_frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  GL_CALL(glShaderSource(compute_frag_shader, 1, &kComputeFragmentShader, nullptr));
  GL_CALL(glCompileShader(compute_frag_shader));

  if (!CheckShaderError(compute_frag_shader)) {
    return false;
  }
  GL_CALL(glAttachShader(gl_.compute_program, compute_shader));
  GL_CALL(glAttachShader(gl_.compute_program, compute_frag_shader));

  const char* varyings[] = {"newPosition"};
  GL_CALL(glTransformFeedbackVaryings(gl_.compute_program, 1, varyings, GL_SEPARATE_ATTRIBS));


  GL_CALL(glLinkProgram(gl_.compute_program));
  GL_CALL(glValidateProgram(gl_.compute_program));
  GL_CALL(glDeleteShader(compute_shader));
  GL_CALL(glDeleteShader(compute_frag_shader));

  if (!CheckLinkError(gl_.compute_program)) {
    return false;
  }


  gl_.render_program = glCreateProgram();

  auto vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  GL_CALL(glShaderSource(vertex_shader, 1, &kVertexShader, nullptr));
  GL_CALL(glCompileShader(vertex_shader));

  if (!CheckShaderError(vertex_shader)) {
    return false;
  }

  auto fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  GL_CALL(glShaderSource(fragment_shader, 1, &kFragmentShader, nullptr));
  GL_CALL(glCompileShader(fragment_shader));

  if (!CheckShaderError(fragment_shader)) {
    return false;
  }

  GL_CALL(glAttachShader(gl_.render_program, vertex_shader));
  GL_CALL(glAttachShader(gl_.render_program, fragment_shader));
  GL_CALL(glLinkProgram(gl_.render_program));
  GL_CALL(glValidateProgram(gl_.render_program));
  GL_CALL(glDeleteShader(vertex_shader));
  GL_CALL(glDeleteShader(fragment_shader));

  GL_CALL(glUseProgram(gl_.compute_program));
  GL_CALL(gl_.old_position_location = glGetAttribLocation(gl_.compute_program, "oldPosition"));
  GL_CALL(gl_.velocity_location = glGetAttribLocation(gl_.compute_program, "oldVelocity"));
  GL_CALL(gl_.delta_location = glGetUniformLocation(gl_.compute_program, "deltaTime"));
  GL_CALL(gl_.canvas_dimensions_location = glGetUniformLocation(gl_.compute_program, "canvasDimensions"));

  GL_CALL(glUseProgram(gl_.render_program));
  GL_CALL(gl_.matrix_location = glGetUniformLocation(gl_.render_program, "transform"));
  GL_CALL(gl_.position_location = glGetAttribLocation(gl_.render_program, "position"));
  GL_CALL(gl_.mouse_pos_location = glGetUniformLocation(gl_.compute_program, "mousePos"));

  GL_CALL(glEnable(GL_PROGRAM_POINT_SIZE));

  return true;
}

void Simulation::GenerateParticles(unsigned int count) {
  particles_count_ = count;
  positions_.reserve(count * 3);
  velocities_.reserve(count * 2);

//  positions_ = {500, 500};
//  velocities_ = {0.1, 0.1};


  auto randomPosFloat = [engine = std::default_random_engine{std::random_device{}()}, distribution = std::uniform_real_distribution<float>()]
      (float lower_bound, float upper_bound) mutable {
        distribution.param(std::uniform_real_distribution<float>::param_type(lower_bound, upper_bound));
        return distribution(engine);
      };

  auto randomInt = [engine = std::default_random_engine{std::random_device{}()}, distribution = std::uniform_int_distribution<int>()]
      (int lower_bound, int upper_bound) mutable {
        distribution.param(std::uniform_int_distribution<int>::param_type(lower_bound, upper_bound));
        return distribution(engine);
      };


  for (unsigned i = 0; i < count; i++) {
    positions_.push_back(randomPosFloat(width_ / 2.0 - 100, width_ / 2.0 + 100));
    positions_.push_back(randomPosFloat(height_ / 2.0 - 100, height_ / 2.0 + 100));
    // position.z is used for point size
    positions_.push_back(randomInt(1, 8));
    velocities_.push_back(randomPosFloat(-300, 300));
    velocities_.push_back(randomPosFloat(-300, 300));

    auto a = velocities_[2 * i];
    auto b = velocities_[2 * i + 1];
    positions_.push_back(std::sqrt(std::pow(a, 2) + std::pow(b, 2)));
  }

  GL_CALL(glGenBuffers(1, &gl_.new_position_buffer));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, gl_.new_position_buffer));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, positions_.size() * sizeof(GLfloat), positions_.data(), GL_DYNAMIC_DRAW));

  GL_CALL(glGenBuffers(1, &gl_.old_position_buffer));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, gl_.old_position_buffer));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, positions_.size() * sizeof(GLfloat), positions_.data(), GL_DYNAMIC_DRAW));

  GL_CALL(glGenBuffers(1, &gl_.velocity_buffer));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, gl_.velocity_buffer));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, velocities_.size() * sizeof(GLfloat), velocities_.data(), GL_STATIC_DRAW));


  struct data {
    GLuint buffer, location, size;
  };

  auto createVao = [](std::vector<data> data) {
    GLuint vao;
    GL_CALL(glGenVertexArrays(1, &vao));
    GL_CALL(glBindVertexArray(vao));
    for (auto d : data) {
      GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, d.buffer));
      GL_CALL(glEnableVertexAttribArray(d.location));
      GL_CALL(glVertexAttribPointer(d.location, d.size, GL_FLOAT, false, 0, 0));
    }

    return vao;
  };
   gl_.position1_compute_vao = createVao({{gl_.old_position_buffer, gl_.old_position_location, 4},
                                         {gl_.velocity_buffer, gl_.velocity_location, 2}});
   gl_.position2_compute_vao = createVao({{gl_.new_position_buffer, gl_.old_position_location, 4},
                                         {gl_.velocity_buffer, gl_.velocity_location, 2}});
   gl_.position1_draw_vao = createVao({{gl_.old_position_buffer, gl_.old_position_location, 4}});
   gl_.position2_draw_vao = createVao({{gl_.new_position_buffer, gl_.old_position_location, 4}});


   GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
   GL_CALL(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

   GL_CALL(glGenTransformFeedbacks(1, &gl_.transform_feedback1));
   GL_CALL(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gl_.transform_feedback1));
   GL_CALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gl_.old_position_buffer));

   GL_CALL(glGenTransformFeedbacks(1, &gl_.transform_feedback2));
   GL_CALL(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, gl_.transform_feedback2));
   GL_CALL(glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, gl_.new_position_buffer));

   GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
   GL_CALL(glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0));

   current_ = {gl_.position1_compute_vao, gl_.transform_feedback2, gl_.position2_draw_vao};
   next_ = {gl_.position2_compute_vao, gl_.transform_feedback1, gl_.position1_draw_vao};
}

void Simulation::Render(float deltaTime) {
   GL_CALL(glUseProgram(gl_.compute_program));
   GL_CALL(glBindVertexArray(current_.compute));
   GL_CALL(glUniform2f(gl_.canvas_dimensions_location, width_, height_));
   GL_CALL(glUniform1f(gl_.delta_location, deltaTime));

      if (mouse_x_ >= 0 && mouse_x_ < width_ && mouse_y_ >= 0 && mouse_y_ < height_ && mouse_clicked_) {
        GL_CALL(glUniform3f(gl_.mouse_pos_location, mouse_x_, mouse_y_, 1.0));
      } else {
        GL_CALL(glUniform3f(gl_.mouse_pos_location, mouse_x_, mouse_y_, 0.0));
      }

   GL_CALL(glEnable(GL_RASTERIZER_DISCARD));

   GL_CALL(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, current_.tf));
   GL_CALL(glBeginTransformFeedback(GL_POINTS));
   GL_CALL(glDrawArrays(GL_POINTS, 0, particles_count_));
   GL_CALL(glEndTransformFeedback());
   GL_CALL(glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0));

   GL_CALL(glDisable(GL_RASTERIZER_DISCARD));

   GL_CALL(glUseProgram(gl_.render_program));
   GL_CALL(glBindVertexArray(current_.draw));

   GL_CALL(glViewport(0, 0, width_, height_));

   glm::mat4 mat = glm::ortho(0.0f, (float)width_, (float)height_, 0.0f, -1.0f, 1.0f);

   GL_CALL(glUniformMatrix4fv(gl_.matrix_location, 1, GL_FALSE, glm::value_ptr(mat)));
   GL_CALL(glDrawArrays(GL_POINTS, 0, particles_count_));

   std::swap(current_, next_);
   GLenum err = glGetError();
   if(err != GL_NO_ERROR) {
    std::cout << "OpenGL Error: " << err << std::endl;
   }
}
