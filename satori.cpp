#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "headers/shader_s.h"
#include "headers/Camera.h"
#include "headers/stb_image.h"

#define SHADERS_LOC "shaders"
#define ASSETS_LOC "assets"

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
unsigned int loadTexture(const char *path);
unsigned int generateColorRamp(const float *data, int nColor);

float mixVal = 0.35f;
float zoomVal = 1.0f;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// control mode
unsigned int mode = 1;

//light
glm::vec3 lightPos(0.2f, 0.1f, 2.5f);

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // glfwSetCursorPosCallback(window, mouse_callback);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
  glViewport(0, 0, 800, 600);

  // compile vertex and fragmentShader first
  Shader satoriShader(SHADERS_LOC "/satori.vs", SHADERS_LOC "/satori.fs");
  Shader lightCubeShader(SHADERS_LOC "/light_cube.vs", SHADERS_LOC "/light_cube.fs");

  // preparing datas to draw
  float cube_vertices[] = {
      // positions          // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // xyz, xy
  };

  int numOfVertexData = 5;
  int numOfTriangles = sizeof(cube_vertices) / (sizeof(cube_vertices[0]) * numOfVertexData);

  unsigned int cubeVBO, cubeVAO;
  glGenBuffers(1, &cubeVBO);
  glGenVertexArrays(1, &cubeVAO);

  //cube VAO/VBO
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

  // position attribute
  int blockSize = 5;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // tex attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // light VAO
  unsigned int lightCubeVAO;
  glGenVertexArrays(1, &lightCubeVAO);
  glBindVertexArray(lightCubeVAO);
  // we only need to bind to the VBO, the container's VBO's data already contains the data.
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  // set the vertex attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // load textures (we now use a utility function to keep the code more organized)
  // -----------------------------------------------------------------------------
  stbi_set_flip_vertically_on_load(true);
  unsigned int diffuseMap = loadTexture(ASSETS_LOC "/sprite_0093.png");
  unsigned int normalMap = loadTexture(ASSETS_LOC "/sprite_0093_n.png");

  // Color Ramp
  float colorRampData[] = {
      0.54f, 0.40f, 0.39f, 1.0f,
      0.73f, 0.54f, 0.52f, 1.0f,
      0.73f, 0.54f, 0.52f, 1.0f,
      0.93f, 0.76f, 0.72f, 1.0f, //rgba
  };
  int nColor = sizeof(colorRampData) / (sizeof(float) * 4);
  unsigned int colorRamp = generateColorRamp(colorRampData, nColor);

  // shader configuration
  // --------------------
  satoriShader.use();
  satoriShader.setInt("material.diffuse", 0);
  satoriShader.setInt("material.normal", 1);
  satoriShader.setInt("material.colorRamp", 2);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float timeValue = glfwGetTime();
    deltaTime = timeValue - lastFrame;
    lastFrame = timeValue;

    // view and projection for all object
    glm::mat4 model;
    glm::mat4 view;
    view = camera.GetViewMatrix();
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //Cubes
    satoriShader.use();
    satoriShader.setMat4("view", view);
    satoriShader.setMat4("projection", projection);
    satoriShader.setVec3("viewPos", camera.Position);

    glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);

    satoriShader.setVec3("light.position", lightPos);
    satoriShader.setVec3("light.ambient", ambientColor);
    satoriShader.setVec3("light.diffuse", diffuseColor);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, colorRamp);

    glBindVertexArray(cubeVAO);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
    satoriShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, numOfTriangles);

    // light cube
    lightCubeShader.use();
    lightCubeShader.setMat4("view", view);
    lightCubeShader.setMat4("projection", projection);

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.01f));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    lightCubeShader.setMat4("model", model);

    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &cubeVAO);
  glDeleteBuffers(1, &cubeVBO);

  glfwTerminate();
  return 0;
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
  {
    mixVal += 0.05f;
    if (mixVal >= 1.0f)
      mixVal = 1.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
  {
    mixVal -= 0.05f;
    if (mixVal <= 0.0f)
      mixVal = 0.0f;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
  {
    zoomVal += 0.01f;
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
  {
    zoomVal -= 0.01f;
  }

  // switching control mode
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
  {
    mode = 1;
    camera.Position = glm::vec3(0.0f, 0.0f, 3.0f);
  }
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
  {
    mode = 2;
    lightPos = glm::vec3(0.2f, 0.1f, 2.5f);
  }

  float speed = 0.3f * deltaTime;
  switch (mode)
  {
  case 1:
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      lightPos += glm::vec3(0.0f, speed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      lightPos += glm::vec3(0.0f, -speed, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      lightPos += glm::vec3(-speed, 0.0f, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      lightPos += glm::vec3(speed, 0.0f, 0.0f);
    break;
  default:
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
      camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
      camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
      camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
      camera.ProcessKeyboard(ASCEND, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      camera.ProcessKeyboard(DESCEND, deltaTime);
    break;
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
  if (firstMouse) // initially set to true
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(const char *path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

unsigned int generateColorRamp(const float *data, int nColor)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  glBindTexture(GL_TEXTURE_1D, textureID);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, nColor, 0, GL_RGBA, GL_FLOAT, data);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  return textureID;
}