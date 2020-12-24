#pragma once


#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "result.hpp"


struct Texture
{
    GLuint id;
};


internal Result<Texture>
Texture_load(const char *path)
{
    int width,
        height,
        nr_channels;

    stbi_set_flip_vertically_on_load(true);
    u8 *data = stbi_load(path, &width, &height, &nr_channels, 0);
    if (data == nullptr)
    {
        return {.ok = false};
    }

    GLuint color_format = nr_channels == 3 ? GL_RGB : GL_RGBA;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        color_format,
        width,
        height,
        0,
        color_format,
        GL_UNSIGNED_BYTE,
        data
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return {
        .ok = true,
        .value = {texture},
    };
}


internal void
Texture_use(Texture *texture, GLuint unit)
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}


