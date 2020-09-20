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

int blockSize;
unsigned int squareVBO;
unsigned int buildSquareVAO();

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
glm::vec3 sunPos;

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
  Shader terrainBgShader(SHADERS_LOC "tex.vs", SHADERS_LOC "tex2.fs");
  Shader terrainFgShader(SHADERS_LOC "tex.vs", SHADERS_LOC "tex2.fs");

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
  blockSize = 5;
  int numOfTriangles = sizeof(square) / (sizeof(square[0]) * blockSize);
  glGenBuffers(1, &squareVBO);
  glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

  unsigned int satoriVAO = buildSquareVAO();
  unsigned int sunVAO = buildSquareVAO();
  unsigned int terrainBgVAO = buildSquareVAO();
  unsigned int terrainFgVAO = buildSquareVAO();

  // load textures (we now use a utility function to keep the code more organized)
  // -----------------------------------------------------------------------------
  stbi_set_flip_vertically_on_load(true);
  unsigned int diffuseMap = loadTexture(ASSETS_LOC "satori_sprout.png");
  unsigned int normalMap = loadTexture(ASSETS_LOC "satori_sprout_n.png");
  unsigned int sun = loadTexture(ASSETS_LOC "sun.png");
  unsigned int terrainFg = loadTexture(ASSETS_LOC "terrain_bg.png");
  unsigned int terrainBg = loadTexture(ASSETS_LOC "terrain_fg.png");

  // Color Ramp
  float lightRampData[] = {
      0.5f, 0.3f, 0.3f, 1.0f,
      0.55f, 0.35f, 0.35f, 1.0f,
      0.6f, 0.4f, 0.4f, 1.0f,
      1.0f, 1.0f, 1.0f, 1.0f, //rgba
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
  terrainBgShader.use();
  terrainBgShader.setInt("material.diffuse", 4);
  terrainFgShader.use();
  terrainFgShader.setInt("material.diffuse", 5);

  // allshader in one array

  Shader allShaders[] = {satoriShader, sunShader, terrainBgShader, terrainFgShader};
  int numOfShaders = sizeof(allShaders) / sizeof(allShaders[0]);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    float timeValue = glfwGetTime();
    deltaTime = timeValue - lastFrame;
    lastFrame = timeValue;

    float dayFreq = 0.5f;
    float globalAmbient = 0.3f + (0.7f * fmax(sin(dayFreq * timeValue), 0.0f));

    glm::vec4 sky = glm::vec4(0.43f, 0.67f, 0.79f, 1.0f) * globalAmbient;
    glClearColor(sky.r, sky.g, sky.b, sky.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    // sun circular motion
    glm::vec2 origin(0.0f, -2.0f);
    float r = 3.0f;
    float circX = origin.x + (r * cos(dayFreq * timeValue));
    float circY = origin.y + (r * sin(dayFreq * timeValue));
    sunPos = glm::vec3(circX, circY, 1.9f);

    // view and projection for all object
    view = camera.GetViewMatrix();
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    for (int i = 0; i < numOfShaders; i++)
    {
      allShaders[i].use();
      allShaders[i].setMat4("view", view);
      allShaders[i].setMat4("projection", projection);
    }

    // sun
    sunShader.use();

    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.25f));
    model = glm::translate(model, sunPos);
    sunShader.setMat4("model", model);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, sun);

    glBindVertexArray(sunVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // terrainBg
    terrainBgShader.use();
    model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(4.0f, 1.0f, 1.0f));
    model = glm::translate(model, glm::vec3(0.0f, -0.75f, 1.0f));
    terrainBgShader.setMat4("model", model);
    terrainBgShader.setFloat("light.ambient", globalAmbient);
    

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, terrainBg);

    glBindVertexArray(terrainBgVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    //Cubes
    satoriShader.use();
    satoriShader.setVec3("light.position", sunPos);
    satoriShader.setFloat("light.ambient", globalAmbient);
    satoriShader.setFloat("light.intensity", fmax(sin(dayFreq * timeValue), 0.0f));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_1D, lightRamp);

    glBindVertexArray(satoriVAO);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.035f, 0.0f, 2.0f));
    satoriShader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, numOfTriangles);

    // terrainFg
    terrainFgShader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.95f, 2.1f));
    terrainFgShader.setMat4("model", model);
    terrainFgShader.setFloat("light.ambient", globalAmbient);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, terrainFg);

    glBindVertexArray(terrainFgVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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
    // sunPos = glm::vec3(0.2f, 0.1f, 2.5f);
  }

  float speed = 0.3f * deltaTime;
  switch (mode)
  {
  case 1:
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //   sunPos += glm::vec3(0.0f, speed, 0.0f);
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //   sunPos += glm::vec3(0.0f, -speed, 0.0f);
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //   sunPos += glm::vec3(-speed, 0.0f, 0.0f);
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //   sunPos += glm::vec3(speed, 0.0f, 0.0f);
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

unsigned int buildSquareVAO()
{
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  return VAO;
}