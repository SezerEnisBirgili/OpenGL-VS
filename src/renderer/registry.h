#pragma once

#include "entities/Hierarchy.h"

#include "renderer/mesh.h" 
#include "renderer/model.h"
#include "renderer/material.h"
#include <unordered_map>

enum class ResourceType { Model, Mesh, Material };

class Registry {
public:
    int loadModel(std::string path);
    Mesh& mesh(int id);
    Texture& texture(int id);

    Mesh* add(const std::string& key, const Mesh& mesh);
    Mesh* get(const std::string& key);
    bool contains(const std::string& key, const Mesh& mesh);
    std::string makeKey(ResourceType type, std::string path, unsigned index = 0);
    
private:
    Hierarchy hierarchy;
    
    std::unordered_map<std::string, int> paths;
    std::unordered_map<int, Model> models;
    std::unordered_map<int, Mesh> meshes;
    std::unordered_map<int, Material> materials;

    int pathToId(const std::string& key);
};