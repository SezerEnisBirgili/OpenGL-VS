#pragma once

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <stb_image.h>

enum class TextureType
{
	NONE,
	DIFFUSE,
	SPECULAR
};

struct Texture
{
    int width = 0;
    int height = 0;
    unsigned int textureID = 0;
    TextureType type = TextureType::NONE;
};
