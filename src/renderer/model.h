#pragma once

#include <vector>

#include "renderer/transform.h"
#include "renderer/material.h"
#include "renderer/mesh.h"

struct ModelNode 
{
    Mesh* mesh;
    Material material;
    Transform transform;
    int parent;
};

struct Model
{
    std::string directory;
    std::vector<ModelNode> nodes;
};

