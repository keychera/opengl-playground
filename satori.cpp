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

#define SHADERS_LOC "shaders/"
#define ASSETS_LOC "assets/"

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
const unsigned int SCR_HEIGHT = 800;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// control mode
unsigned int mode = 1;

//light
glm::vec3 lightPos(0.1f, 0.0f, 1.0f);

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
  glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

  // compile vertex and fragmentShader first
  Shader satoriShader(SHADERS_LOC "tex.vs", SHADERS_LOC "satori.fs");
  Shader sunShader(SHADERS_LOC "tex.vs", SHADERS_LOC "tex.fs");
  Shader terrainBgShader(SHADERS_LOC "tex.vs", SHADERS_LOC "tex.fs");
  Shader terrainFgShader(SHADERS_LOC "tex.vs", SHADERS_LOC "tex.fs");

  // preparing datas to draw
  float square[] = {
      // positions          // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // xyz, xy
  };

  int numOfVertexData = 5;
  int numOfTriangles = sizeof(square) / (sizeof(square[0]) * numOfVertexData);

  unsigned int squareVBO, satoriVAO;
  glGenBuffers(1, &squareVBO);
  glGenVertexArrays(1, &satoriVAO);

  //satori VAO/VBO
  glBindVertexArray(satoriVAO);
  glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

  // position attribute
  int blockSize = 5;
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  // tex attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // light VAO
  unsigned int sunVAO;
  glGenVertexArrays(1, &sunVAO);
  glBindVertexArray(sunVAO);
  // we only need to bind to the VBO, the container's VBO's data already contains the data.
  glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
  // set the vertex attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  // load textures (we now use a utility function to keep the code more organized)
  // -----------------------------------------------------------------------------
  stbi_set_flip_vertically_on_load(true);
  unsigned int diffuseMap = loadTexture(ASSETS_LOC "satori_sprout.png");
  unsigned int normalMap = loadTexture(ASSETS_LOC "satori_sprout_n.png");
  unsigned int sun = loadTexture(ASSETS_LOC "sun.png");

  // Color Ramp
  float lightRampData[] = {
      1.0f, 1.0f, 1.0f, 1.0f,
      0.6f, 0.4f, 0.4f, 1.0f,
      0.55f, 0.35f, 0.35f, 1.0f,
      0.5f, 0.3f, 0.3f, 1.0f, //rgba
  };
  int nColor = sizeof(lightRampData) / (sizeof(float) * 4);
  unsigned int lightRamp = generateColorRamp(lightRampData, nColor);

  // shader configuration
  // --------------------
  satoriShader.use();
  satoriShader.setInt("material.diffuse", 0);
  satoriShader.setInt("material.normal", 1);
  satoriShader.setInt("light.colorRamp", 2);

  sunShader.use();
  sunShader.setInt("material.diffuse", 3);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    glClearColor(0.43f, 0.67f, 0.79f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float timeValue = glfwGetTime();
    deltaTime = timeValue - lastFrame;
    lastFrame = timeValue;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    // lightPos circular motion
    float r = 0.3f;
    float freq = 0.5f;
    float circX = r * cos(freq * timeValue);
    float circY = r * sin(freq * timeValue);
    lightPos = glm::vec3(circX, circY, 1.9f);

    // light cube
    sunShader.use();
    sunShader.setMat4("view", view);
    sunShader.setMat4("projection", projection);

    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.1f));
    sunShader.setMat4("model", model);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sun);

    glBindVertexArray(sunVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // view and projection for all object
    view = camera.GetViewMatrix();
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    //Cubes
    satoriShader.use();
    satoriShader.setMat4("view", view);
    satoriShader.setMat4("projection", projection);

    glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
    glm::vec3 diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);

    satoriShader.setVec3("light.position", lightPos);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, lightRamp);

    glBindVertexArray(satoriVAO);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 2.0f));
    satoriShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, numOfTriangles);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteVertexArrays(1, &satoriVAO);
  glDeleteBuffers(1, &squareVBO);

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
    // lightPos = glm::vec3(0.2f, 0.1f, 2.5f);
  }

  float speed = 0.3f * deltaTime;
  switch (mode)
  {
  case 1:
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //   lightPos += glm::vec3(0.0f, speed, 0.0f);
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //   lightPos += glm::vec3(0.0f, -speed, 0.0f);
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //   lightPos += glm::vec3(-speed, 0.0f, 0.0f);
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //   lightPos += glm::vec3(speed, 0.0f, 0.0f);
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