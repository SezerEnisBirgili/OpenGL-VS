#pragma once

#include <vector>
#include <glm/glm.hpp>

struct RenderItem
{
    const Mesh* mesh;
    const Material* material;
    glm::mat4       world;
    int             entity;
};
