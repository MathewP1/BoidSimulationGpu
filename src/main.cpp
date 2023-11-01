#include <iostream>
#include <chrono>


#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "config.h"
#include "gl_util.h"
#include "simulation.h"

constexpr float kWindowWidth = 1500;
constexpr float kWindowHeight = 1200;

int main(void) {
  GLFWwindow* window;

  /* Initialize the library */
  if (!glfwInit()) return -1;

  // Set lowest possible version
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  // Set to core profile - necessary on MacOS
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  /* Create a windowed mode window and its OpenGL context */
  window =
      glfwCreateWindow(kWindowWidth, kWindowHeight, "Hello World", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  // Init glew to link to OpenGL implementation
  if (glewInit() != GLEW_OK) {
    std::cout << "Error!" << std::endl;
  }

  // set up OpenGL options
  GL_CALL(glEnable(GL_BLEND));
  GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  // clear with gray so that black object are visible
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "Resources directory: " << RESOURCE_PATH << std::endl;

  // ImGUI init
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  const char* glsl_version = "#version 410";  // should match OpenGL 4.1
  ImGui_ImplOpenGL3_Init(glsl_version);
  ImGui::StyleColorsDark();

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);


  // init simulation
  Simulation sim(width, height);
  sim.SetupGl();
  sim.GenerateParticles(100000);


  auto last_timestamp = std::chrono::high_resolution_clock::now();
  unsigned int frame_count = 0;
  unsigned int fps = 0;
  auto accumulated_time = std::chrono::duration<float>(0);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    /* Render here */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // calculate FPS
    auto current_timestamp = std::chrono::high_resolution_clock::now();
    auto delta = current_timestamp - last_timestamp;
    accumulated_time += delta;
    auto delta_sec = std::chrono::duration<float>(delta).count();

    // Calculate FPS
    if (accumulated_time >= std::chrono::seconds(1)) {
      fps = frame_count;
      accumulated_time -= std::chrono::seconds(1);  // instead of resetting to zero
      frame_count = 0;
    }
    frame_count++;

    last_timestamp = current_timestamp;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    std::cout << width << " " << height << std::endl;
    sim.SetCanvasDimensions(width, height);

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    sim.SetMousePos(x, y);

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    sim.SetMouseClicked(state == GLFW_PRESS);

    // Begin rendering scene here
    sim.Render(delta_sec);
    // End rendering scene here

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Scene");
    // Render UI elements here
    ImGui::Text("FPS: %i", fps);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  return 0;
}