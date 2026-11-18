#pragma once

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

struct Texture {
    unsigned int id = 0;
    std::string path;

    Texture() = default;
    ~Texture() {
        if (id != 0) glDeleteTextures(1, &id);
    }

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept : id(other.id), path(std::move(other.path)) {
        other.id = 0;
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            if (id != 0) glDeleteTextures(1, &id);
            id = other.id;

            path = std::move(other.path);
            other.id = 0;
        }
        return *this;
    }

    void bind(unsigned int slot = 0) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, id);
    }
};

inline unsigned int loadTextureFromFile(const std::string& filePath, bool flipVertically = true) {
    stbi_set_flip_vertically_on_load(flipVertically);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (!data) {
        std::cerr << "Texture failed to load at path: " << filePath << "\n";
        return 0;
    }

    GLenum format = GL_RGB;
    if (nrComponents == 1) format = GL_RED;
    else if (nrComponents == 3) format = GL_RGB;
    else if (nrComponents == 4) format = GL_RGBA;

    unsigned int textureID = 0;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return textureID;
}

