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
const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// system
std::function<void()> loop;
void main_loop() { loop(); }

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 sunPos;

int loadTextureSDL(const char *filename);
unsigned int generateColorRamp(const float *data, int nColor);

int blockSize = 5;
float square[] = {
    // positions          // texture coords
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // xyz, xy
};
unsigned int buildSquareVAO();

// extern
#define jsfunc extern "C" void __attribute__((used))

int main()
{
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(SCR_WIDTH, SCR_HEIGHT, 0, &window, nullptr);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // compile vertex and fragmentShader
    Shader satoriShader(SHADERS_LOC "/tex.vs", SHADERS_LOC "/satori.fs");
    Shader sunShader(SHADERS_LOC "/tex.vs", SHADERS_LOC "/tex.fs");
    Shader terrainBgShader(SHADERS_LOC "/tex.vs", SHADERS_LOC "/tex2.fs");
    Shader terrainFgShader(SHADERS_LOC "/tex.vs", SHADERS_LOC "/tex2.fs");

    // preparing VAOs
    unsigned int satoriVAO = buildSquareVAO();
    unsigned int sunVAO = buildSquareVAO();
    unsigned int terrainBgVAO = buildSquareVAO();
    unsigned int terrainFgVAO = buildSquareVAO();

    // load textures
    unsigned int diffuseMap = loadTextureSDL(ASSETS_LOC "/satori_sprout.png");
    unsigned int normalMap = loadTextureSDL(ASSETS_LOC "/satori_sprout_n.png");
    unsigned int sun = loadTextureSDL(ASSETS_LOC "/sun.png");
    unsigned int terrainFg = loadTextureSDL(ASSETS_LOC "/terrain_bg.png");
    unsigned int terrainBg = loadTextureSDL(ASSETS_LOC "/terrain_fg.png");

    // Color Ramp
    float lightRampData[] = {
        0.5f, 0.3f, 0.3f, 1.0f,
        0.55f, 0.35f, 0.35f, 1.0f,
        0.6f, 0.4f, 0.4f, 1.0f,
        0.8f, 0.7f, 0.7f, 1.0f,
        0.9f, 0.8f, 0.8f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f, //rgba
    };
    int nColor = sizeof(lightRampData) / (sizeof(float) * 4);
    unsigned int lightRamp = generateColorRamp(lightRampData, nColor);

    // shader configuration
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
    int numOfTriangles = sizeof(square) / (sizeof(square[0]) * blockSize);

    // gl settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    loop = [&] {
        float timeValue = SDL_GetTicks();
        deltaTime = timeValue - lastFrame;
        lastFrame = timeValue;

        float dayFreq = 0.5f;
        float globalAmbient = 0.3f + (0.7f * fmax(sin(dayFreq * timeValue), 0.0f));

        glm::vec4 sky = glm::vec4(0.43f, 0.67f, 0.79f, 1.0f) * globalAmbient;
        glClearColor(sky.r, sky.g, sky.b, sky.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view and projection for all object
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
        view = camera.GetViewMatrix();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        for (int i = 0; i < numOfShaders; i++)
        {
            allShaders[i].use();
            allShaders[i].setMat4("view", view);
            allShaders[i].setMat4("projection", projection);
        }

        // sun circular motion
        glm::vec2 origin(0.0f, -2.0f);
        float r = 3.0f;
        float circX = origin.x + (r * cos(dayFreq * timeValue));
        float circY = origin.y + (r * sin(dayFreq * timeValue));
        sunPos = glm::vec3(circX, circY, 1.9f);

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

        // satori
        satoriShader.use();
        satoriShader.setVec3("light.position", sunPos);
        satoriShader.setFloat("light.ambient", globalAmbient);
        satoriShader.setFloat("light.intensity", fmax(sin(dayFreq * timeValue), 0.0f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, lightRamp);

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

unsigned int generateColorRamp(const float *data, int nColor)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    // simulate 1d texture using 2d texture with height 1
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nColor, 1, 0, GL_RGBA, GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureID;
}

unsigned int buildSquareVAO()
{
    unsigned int VAO, VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, blockSize * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    return VAO;
}