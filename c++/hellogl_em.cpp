#include <functional>

#define __EMSCRIPTEN__ 1
#include <emscripten/emscripten.h>
#include <SDL.h>
#include <SDL_image.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "headers/shader_s.h"
#include "headers/Camera.h"

#define SHADERS_LOC "emscripten/shaders"
#define ASSETS_LOC "emscripten/assets"

// settings
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// system
std::function<void()> loop;
void main_loop() { loop(); }

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

int loadTextureSDL(const char *filename);

// extern
#define jsfunc extern "C" void __attribute__((used))
jsfunc control_forward()
{
    camera.ProcessKeyboard(FORWARD, deltaTime);
}
jsfunc control_left()
{
    camera.ProcessKeyboard(LEFT, deltaTime);
}
jsfunc control_backward()
{
    camera.ProcessKeyboard(BACKWARD, deltaTime);
}
jsfunc control_right()
{
    camera.ProcessKeyboard(RIGHT, deltaTime);
}
jsfunc control_ascend()
{
    camera.ProcessKeyboard(ASCEND, deltaTime);
}
jsfunc control_descend()
{
    camera.ProcessKeyboard(DESCEND, deltaTime);
}

int main()
{
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, 0, &window, nullptr);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // preparing datas to draw
    float cube_vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f // for formatting
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)};

    unsigned int cubeVBO;
    glGenBuffers(1, &cubeVBO);

    //cube VAO/VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    // position attribute
    int blockSize = 8;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // tex attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // compile vertex and fragmentShader
    Shader cubeShader(SHADERS_LOC "/cube.vs", SHADERS_LOC "/light.fs");

    // load textures
    // -----------------------------------------------------------------------------
    unsigned int diffuseMap = loadTextureSDL(ASSETS_LOC "/container2.png");
    unsigned int specularMap = loadTextureSDL(ASSETS_LOC "/container2_specular.png");

    // shader configuration
    // --------------------
    cubeShader.use();
    cubeShader.setInt("material.diffuse", 0);
    cubeShader.setInt("material.specular", 1);

    glEnable(GL_DEPTH_TEST);

    camera.MovementSpeed = 0.006f;

    loop = [&] {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float timeValue = SDL_GetTicks();
        deltaTime = timeValue - lastFrame;
        lastFrame = timeValue;
        // view and projection for all object
        glm::mat4 model;
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection;
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        //Cubes
        cubeShader.use();
        cubeShader.setMat4("view", view);
        cubeShader.setMat4("projection", projection);
        cubeShader.setVec3("viewPos", camera.Position);

        glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);
        glm::vec3 diffuseColor = glm::vec3(0.8f, 0.8f, 0.8f);

        cubeShader.setVec3("light.position", camera.Position);
        cubeShader.setVec3("light.direction", camera.Front);
        cubeShader.setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));
        cubeShader.setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));

        cubeShader.setVec3("light.ambient", ambientColor);
        cubeShader.setVec3("light.diffuse", diffuseColor);
        cubeShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        cubeShader.setFloat("light.constant", 1.0f);
        cubeShader.setFloat("light.linear", 0.09f);
        cubeShader.setFloat("light.quadratic", 0.032f);

        cubeShader.setFloat("material.shininess", 32.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularMap);

        for (unsigned int i = 0; i < 10; i++)
        {
            float angle = 20.0f * i;
            model = glm::mat4(1.0f);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            // model = glm::rotate(model, (float)timeValue * glm::radians(50.0f), glm::vec3(0.5f, sin(timeValue) * 0.5f, 0.0f));
            model = glm::translate(model, cubePositions[i]);
            cubeShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        SDL_GL_SwapWindow(window);
    };

    emscripten_set_main_loop(main_loop, 0, true);

    return EXIT_SUCCESS;
}

int loadTextureSDL(const char *filename)
{
    // source: https://gist.github.com/mortennobel/0e9e90c9bbc61cc99d5c3e9c038d8115
    unsigned int texture;

    /* Create storage space for the texture */
    SDL_Surface *image;
    image = IMG_Load(filename);
    if (image)
    {
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        std::cout << "Loaded " << image->w << " " << image->h << std::endl;

        // Enforce RGB / RGBA
        GLenum format;
        SDL_Surface *formattedImage;
        if (image->format->BytesPerPixel == 3)
        {

            formattedImage = SDL_ConvertSurfaceFormat(image,
                                                      SDL_PIXELFORMAT_RGB24,
                                                      0);
            format = GL_RGB;
        }
        else
        {
            formattedImage = SDL_ConvertSurfaceFormat(image,
                                                      SDL_PIXELFORMAT_RGBA32,
                                                      0);
            format = GL_RGBA;
        }

        /* Generate The Texture */
        glTexImage2D(GL_TEXTURE_2D, 0, format, formattedImage->w,
                     formattedImage->h, 0, format,
                     GL_UNSIGNED_BYTE, formattedImage->pixels);

        /* Linear Filtering */
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
    {
        std::cout << "Cannot load " << filename << ",Error: " << IMG_GetError() << std::endl;
    }
    SDL_FreeSurface(image);
    return texture;
}
