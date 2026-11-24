#pragma once

#include <vector>

#include "renderer/transform.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

struct ModelNode 
{
    Mesh* mesh;
    Transform transform;
};

struct Model
{
    std::string directory;
    Material material;
    Hierarchy hierarchy;
    std::vector<ModelNode> nodes;
};

