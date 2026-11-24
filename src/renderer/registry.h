#pragma once

#include "renderer/shader.h"
#include "renderer/mesh.h" 
#include "renderer/model.h"
#include "renderer/material.h"
#include <unordered_map>

enum class ResourceType { Model, Mesh, Material };

class Registry {
public:
    int loadModel(std::string path);

    Mesh* add(const std::string& key, const Mesh& mesh);
    Model* add(const std::string& key, const Model& mesh);

    Mesh* getMesh(const std::string& key);
    Model* getModel(const std::string& key);
    Shader* getShader(const std::string& key);

    Model* model(int id);
    Shader* shader(int id);

    bool contains(const std::string& key, const Mesh& mesh);
    std::string makeKey(ResourceType type, std::string path, unsigned index = 0);
    
private:
    int nextId = 0;
    
    std::unordered_map<std::string, int> paths;
    std::unordered_map<int, Model> models;
    std::unordered_map<int, Mesh> meshes;
    std::unordered_map<int, Shader> shaders;
    std::unordered_map<int, Material> materials;

    int pathToId(const std::string& key);
};