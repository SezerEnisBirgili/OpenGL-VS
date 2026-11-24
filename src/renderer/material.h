#pragma once

#include "glm/ext/vector_float3.hpp"
#include "renderer/texture.h"

struct Material
{
    Texture diffuse;
    Texture specular;
    int shaderId = 0;

    glm::vec3 color{ 1.0f };
    float shininess   = 32.0f;
    float emissive    = 0.0f;
    float alphaCutoff = 0.5f;
    float alpha       = 1.0f;
    bool  isMasked    = false;
};