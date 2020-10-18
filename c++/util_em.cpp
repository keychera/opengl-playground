#define __EMSCRIPTEN__ 1
#include <SDL_image.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

#include <iostream>
#include <memory>

int invert_image(int width, int height, void *image_pixels)
{
    auto temp_row = std::unique_ptr<char>(new char[width]);
    if (temp_row.get() == nullptr)
    {
        SDL_SetError("Not enough memory for image inversion");
        return -1;
    }
    //if height is odd, don't need to swap middle row
    int height_div_2 = height / 2;
    for (int index = 0; index < height_div_2; index++)
    {
        //uses string.h
        memcpy((Uint8 *)temp_row.get(),
               (Uint8 *)(image_pixels) +
                   width * index,
               width);

        memcpy(
            (Uint8 *)(image_pixels) +
                width * index,
            (Uint8 *)(image_pixels) +
                width * (height - index - 1),
            width);
        memcpy(
            (Uint8 *)(image_pixels) +
                width * (height - index - 1),
            temp_row.get(),
            width);
    }
    return 0;
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
        invert_image(formattedImage->w * formattedImage->format->BytesPerPixel, formattedImage->h, (char *)formattedImage->pixels);

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
