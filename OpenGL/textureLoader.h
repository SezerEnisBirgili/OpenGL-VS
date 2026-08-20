#pragma once
#include <glad/glad.h>
#include "stb_image.h"
#include <iostream>

inline bool loadTexture(const char* texFileName, unsigned int& texture)
{
    std::cout << "loadTexture: " << texFileName << std::endl;

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texFileName, &width, &height, &nrChannels, 0);

    if (!data)
    {
        std::cout << "Failed to load texture '" << texFileName << "': "
            << stbi_failure_reason() << std::endl;
        texture = 0;
        return false;
    }

    GLenum format = GL_RGB;
    if (nrChannels == 1) { format = GL_RED; }
    else if (nrChannels == 3) { format = GL_RGB; }
    else if (nrChannels == 4) { format = GL_RGBA; }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}