#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/noise.hpp>
#include <gtc/type_ptr.hpp>

#include <iostream>
#include <shader_s.h>
#include <vector>

#include <map>
#include <memory>
#include <string>

enum class Screen { MAIN_GAME, SETTINGS };

Screen currentScreen = Screen::MAIN_GAME;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 cameraPosNon = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFrontNon = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUpNon = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool firstMouse = true;
bool mouse_state = false;

float yaw = -89.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float fov = 69.0f;
float speed = 2.0f;
float sens = 0.1f;
float left = 0.0f;
float up = 0.0f;
float down = 0.0f;
bool temp = false;

int width = 256, height = 256;
GLfloat xDiv = (GLfloat)(rand() % 500) + 1750.0f;
GLfloat yDiv = (GLfloat)(rand() % 350) + 1750.0f;
GLfloat xMul = (GLfloat)(rand() % 150) + 75.0f;
GLfloat yMul = (GLfloat)(rand() % 150) + 75.0f;
GLfloat low = 100.0f, high = 0.0f;
int octaves = 2;
GLuint textureID;
std::vector<double> noiseData(width *height);
float frequency = 10.0f;
float amplitude = 1.0f;
GLfloat scale = 5.0f;
std::vector<float> vertices;
GLfloat xScale = 4.0f;
GLfloat zScale = 4.0f;

float baseHeight = 1.0f; // Base height of the terrain
float maxHeight = 10.0f; // Maximum height of the terrain

void updateNoise(float frequency, float amplitude);
void generateVertices();

int main() {
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "graphics engine", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // build and compile our shader program
  // ------------------------------------
  Shader ourShader("src/8.3.cpuheight.vs", "src/8.3.cpuheight.fs");
  glfwSetCursorPosCallback(window, mouse_callback);
  // ImGui setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  // Set up initial noise and vertices
  updateNoise(frequency, amplitude);
  generateVertices();

  // generate TERRAIN VAO
  unsigned int terrainVAO, terrainVBO, terrainEBO;
  glGenVertexArrays(1, &terrainVAO);
  glBindVertexArray(terrainVAO);

  // VBO
  glGenBuffers(1, &terrainVBO);
  glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0],
               GL_STATIC_DRAW);

  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // texture coordinates attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // Indices
  std::vector<unsigned> indices;
  for (unsigned int m = 0; m < height - 1; m++) {
    for (unsigned int j = 0; j < width; j++) {
      for (unsigned k = 0; k < 2; k++) {
        indices.push_back(j + width * (m + k));
      }
    }
  }

  glGenBuffers(1, &terrainEBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
               &indices[0], GL_STATIC_DRAW);

  glEnable(GL_DEPTH_TEST);

  int NUM_STRIPS = height - 1;
  int NUM_VERTS_PER_STRIP = (width) * 2 - 2;
  std::cout << "Created lattice of " << NUM_STRIPS << " strips with "
            << NUM_VERTS_PER_STRIP << " triangles each" << std::endl;
  std::cout << "Created " << NUM_STRIPS * NUM_VERTS_PER_STRIP
            << " triangles total" << std::endl;

  bool drawtriangle = true;
  float size = 1.0f;
  float xAxis = 0.0f;
  float yAxis = 0.0f;
  bool wireMode = false;

  // render loop
  // -----------
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // input
    processInput(window);

    // render
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*
    // Floor and ceil camera (x,z) and find height
    float camx = cameraPos.x / xScale;
    float camz = cameraPos.z / zScale;

    int x1 = (int)floor(camx);
    int z1 = (int)floor(camz);

    int x2 = x1 + 1;
    int z2 = z1 + 1;

    if (x1 >= 0 && x2 < width && z1 >= 0 && z2 < height)
    {
        float h1 = vertices[(z1 + (x1 * width)) * 5 + 1];
        float h2 = vertices[(z1 + (x2 * width)) * 5 + 1];
        float h3 = vertices[(z2 + (x1 * width)) * 5 + 1];
        float h4 = vertices[(z2 + (x2 * width)) * 5 + 1];

        float fracX = camx - x1;
        float fracZ = camz - z1;
        float i1 = h1 * (1 - fracX) + h2 * fracX;
       float i2 = h3 * (1 - fracX) + h4 * fracX;
        float interpolatedHeight = i1 * (1 - fracZ) + i2 * fracZ;

        cameraPos.y = interpolatedHeight + 2.0f;
    }
    else
    {
        cameraPos.y =  3.0f;
    }
    */
    ourShader.use();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // --------------------------------------------------------------------

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection;
    glm::mat4 view;

    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.5f, 1.0f, 0.0f));
    projection =
        glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 10000.0f);
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    int modelLoc = glGetUniformLocation(ourShader.ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    int viewLoc = glGetUniformLocation(ourShader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    int projectionLoc = glGetUniformLocation(ourShader.ID, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // -------------------------------------------------------------------

    switch (currentScreen) {
    case Screen::MAIN_GAME:
      glBindVertexArray(terrainVAO);
      if (drawtriangle) {
        for (unsigned strip = 0; strip < NUM_STRIPS; strip++) {
          glDrawElements(
              GL_TRIANGLE_STRIP, NUM_VERTS_PER_STRIP + 2, GL_UNSIGNED_INT,
              (void *)(sizeof(unsigned) * (NUM_VERTS_PER_STRIP + 2) * strip));
        }
      }

      if (wireMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }

      ImGui::Checkbox("drawtriangle", &drawtriangle);
      ImGui::SliderFloat("size", &size, -0.2f, 4.0f);
      ImGui::SliderFloat("xAxis", &xAxis, -5.0f, 5.5f);
      ImGui::SliderFloat("yAxis", &yAxis, -5.0f, 5.5f);
      ImGui::Checkbox("mouse_state", &mouse_state);
      ImGui::SliderFloat("camera speed", &speed, 50.0f, 1000.0f);
      ImGui::SliderFloat("sensitivity", &sens, 0.01f, 0.5f);
      ImGui::Checkbox("camera view", &temp);
      ImGui::Checkbox("wireMode", &wireMode);

      // Update frequency and noise data
      if (ImGui::SliderFloat("frequency", &frequency, 1.0f, 100.0f)) {
        std::cout << "frequency: " << frequency << std::endl;
        updateNoise(frequency, amplitude);
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (ImGui::SliderFloat("amplitude", &amplitude, 0.0f, 8.0f)) {
        std::cout << "frequency: " << amplitude << std::endl;
        updateNoise(frequency, amplitude);
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (ImGui::SliderFloat("xScale", &xScale, 0.1f, 10.0f)) {
        std::cout << "xScale: " << xScale << std::endl;
        updateNoise(frequency, amplitude);
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (ImGui::SliderFloat("zScale", &zScale, 0.1f, 10.0f)) {
        std::cout << "zScale: " << zScale << std::endl;
        updateNoise(frequency, amplitude);
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (ImGui::SliderFloat("scale", &scale, 0.1f, 10.0f)) {
        std::cout << "scale: " << scale << std::endl;
        updateNoise(frequency, amplitude);
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (ImGui::SliderFloat("Base Height", &baseHeight, 0.0f, 20.0f)) {
        std::cout << "Base Height: " << baseHeight << std::endl;
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (ImGui::SliderFloat("Max Height", &maxHeight, 1.0f, 50.0f)) {
        std::cout << "Max Height: " << maxHeight << std::endl;
        generateVertices();
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
                     &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned),
                     &indices[0], GL_STATIC_DRAW);
      }

      if (mouse_state)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

      ourShader.use();
      ourShader.setFloat("xAxis", xAxis);
      ourShader.setFloat("size", size);
      ourShader.setFloat("yAxis", yAxis);

      if (ImGui::Button("Settings"))
        currentScreen = Screen::SETTINGS;
      break;
    case Screen::SETTINGS:
      ImGui::Begin("Settings");
      ImGui::Text("This is the settings screen!");
      if (ImGui::Button("Back to Main Game"))
        currentScreen = Screen::MAIN_GAME;
      ImGui::End();
      break;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glDeleteVertexArrays(1, &terrainVAO);
  glDeleteBuffers(1, &terrainVBO);
  glDeleteBuffers(1, &terrainEBO);

  glfwTerminate();
  return 0;
}

/*
// Floor and ceil camera (x,z) and find height
float camx = cameraPos.x / xScale;
float camz = cameraPos.z / zScale;

int x1 = (int)floor(camx);
int z1 = (int)floor(camz);

int x2 = x1 + 1;
int z2 = z1 + 1;

if (x1 >= 0 && x2 < width && z1 >= 0 && z2 < height)
{
    float h1 = vertices[(z1 + (x1 * width)) * 5 + 1];
    float h2 = vertices[(z1 + (x2 * width)) * 5 + 1];
    float h3 = vertices[(z2 + (x1 * width)) * 5 + 1];
    float h4 = vertices[(z2 + (x2 * width)) * 5 + 1];

    float fracX = camx - x1;
    float fracZ = camz - z1;
    float i1 = h1 * (1 - fracX) + h2 * fracX;
   float i2 = h3 * (1 - fracX) + h4 * fracX;
    float interpolatedHeight = i1 * (1 - fracZ) + i2 * fracZ;

    cameraPos.y = interpolatedHeight + 2.0f;
}
else
{
    cameraPos.y =  3.0f;
}
*/
// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
  float cameraSpeed = speed * deltaTime;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos +=
        glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    cameraPos += cameraUp * cameraSpeed;

  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    cameraPos -= cameraUp * cameraSpeed;

  static bool prev_key_state = false;
  bool current_key_state = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
  if (current_key_state && !prev_key_state) {
    mouse_state = !mouse_state;
    if (!mouse_state) { // If mouse_state is set to false, reset mouse position
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      glfwSetCursorPos(window, SCR_WIDTH / 2.0,
                       SCR_HEIGHT / 2.0); // Reset mouse position
      firstMouse = true; // Set firstMouse to true to handle the next mouse
                         // movement correctly
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }
  prev_key_state = current_key_state;
}
// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// Update noise data

void updateNoise(float frequency, float amplitude) {
  noiseData.clear();
  low = 0.0f;
  high = 0.0f;
  float xFactor = 1.0f / (width - 1);
  float yFactor = 1.0f / (height - 1);
  for (int r = 0; r < height; ++r) {
    for (int c = 0; c < width; ++c) {
      float x = xFactor * c;
      float y = yFactor * r;
      float sum = 0.0f;
      float freq = frequency;
      for (int o = 0; o < octaves; ++o) {
        sum += glm::perlin(
                   glm::vec2(y * freq / xDiv * xMul, x * freq / yDiv * yMul)) *
               amplitude;
        freq *= 2.0f;
        sum *= scale;
      }
      noiseData.push_back(sum);
      if (sum < low)
        low = sum;
      if (sum > high)
        high = sum;
    }
  }
  // No need to normalize here as we're going to map it in generateVertices
}
// Generate vertex data

void generateVertices() {
  vertices.clear();
  std::vector<float> bottomVertices;

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float x = (float)i * xScale;
      float z = (float)j * zScale;

      // Generate bottom vertices
      bottomVertices.push_back(x);
      bottomVertices.push_back(0.0f); // Bottom plane at height 0
      bottomVertices.push_back(z);
      bottomVertices.push_back((float)j /
                               (float)(width - 1)); // Texture coordinate x
      bottomVertices.push_back((float)i /
                               (float)(height - 1)); // Texture coordinate y

      // Generate top vertices
      float noiseValue = noiseData[i * width + j];
      GLfloat tempY = baseHeight + (noiseValue * maxHeight);
      vertices.push_back(x);
      vertices.push_back(tempY);
      vertices.push_back(z);
      vertices.push_back((float)j / (float)(width - 1)); // Texture coordinate x
      vertices.push_back((float)i /
                         (float)(height - 1)); // Texture coordinate y
    }
  }

  // Combine top and bottom vertices
  vertices.insert(vertices.end(), bottomVertices.begin(), bottomVertices.end());

  // Generate indices for the sides and faces
  std::vector<unsigned> indices;
  for (int i = 0; i < height - 1; i++) {
    for (int j = 0; j < width - 1; j++) {
      int topLeft = i * width + j;
      int topRight = topLeft + 1;
      int bottomLeft = topLeft + width;
      int bottomRight = bottomLeft + 1;

      // Top face
      indices.push_back(topLeft);
      indices.push_back(bottomLeft);
      indices.push_back(topRight);
      indices.push_back(topRight);
      indices.push_back(bottomLeft);
      indices.push_back(bottomRight);

      // Bottom face
      int offset = width * height;
      indices.push_back(topLeft + offset);
      indices.push_back(topRight + offset);
      indices.push_back(bottomLeft + offset);
      indices.push_back(topRight + offset);
      indices.push_back(bottomRight + offset);
      indices.push_back(bottomLeft + offset);

      // Sides
      indices.push_back(topLeft);
      indices.push_back(topLeft + offset);
      indices.push_back(bottomLeft);

      indices.push_back(bottomLeft);
      indices.push_back(topLeft + offset);
      indices.push_back(bottomLeft + offset);

      indices.push_back(topRight);
      indices.push_back(bottomRight);
      indices.push_back(topRight + offset);

      indices.push_back(bottomRight);
      indices.push_back(bottomRight + offset);
      indices.push_back(topRight + offset);
    }
  }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {

  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  if (!mouse_state) {
    // Reset mouse position to the center of the window
    // glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
    // Calculate the offset from the center
    xoffset = 0.0f;
    yoffset = 0.0f;
  }

  xoffset *= sens;
  yoffset *= sens;

  yaw += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;
  // test etst 

  glm::vec3 direction;
  direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  direction.y = sin(glm::radians(pitch));
  direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(direction);
}
