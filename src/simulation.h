//
// Created by Mateusz Palkowski on 01/11/2023.
//

#ifndef OPENGL_CMAKE_SIMULATION_H
#define OPENGL_CMAKE_SIMULATION_H

#include "GL/glew.h"

#include <vector>

class Simulation {
 public:
  Simulation(int canvas_width, int canvas_height) : width_(canvas_width), height_(canvas_height) {}
  void SetCanvasDimensions(int canvas_width, int canvas_height) {
    width_ = canvas_width;
    height_ = canvas_height;
  }
  void SetMousePos(float x, float y) {
    mouse_x_ = x;
    mouse_y_ = y;
  }

  void SetMouseClicked(bool m) {
    mouse_clicked_ = m;
  }
  bool SetupGl();
  void GenerateParticles(unsigned int count);
  void Render(float deltaTime);

 private:
  int width_, height_;
  float mouse_x_, mouse_y_;
  bool mouse_clicked_;
  struct Gl {
    GLuint compute_program, render_program;
    GLuint old_position_location, velocity_location;
    GLuint new_position_location, position_location, matrix_location;
    GLuint delta_location, canvas_dimensions_location, mouse_pos_location;
    GLuint position1_compute_vao, position2_compute_vao, position1_draw_vao, position2_draw_vao;
    GLuint transform_feedback1, transform_feedback2;
    GLuint new_position_buffer, old_position_buffer, velocity_buffer;
  };

  struct State {
    GLuint compute, tf, draw;
  };
  Gl gl_{};
  State current_, next_;
  std::vector<GLfloat > positions_;
  std::vector<GLfloat> velocities_;
  unsigned particles_count_ = 0;
};

#endif  // OPENGL_CMAKE_SIMULATION_H
