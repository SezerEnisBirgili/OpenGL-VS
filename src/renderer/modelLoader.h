#pragma once

#include "assimp/scene.h"

#include "renderer/model.h"
#include "renderer/registry.h"
#include "renderer/vertex.h"


struct ModelLoader
{
public:
    Model model;
    std::string path;
    std::unordered_map<std::string, Texture> loadedTextures;

    ModelLoader(Registry& reg, const std::string& p) : registry(reg) {
        path = p;
    }

    Model* loadModel();

private:
    Registry& registry;

    void processNode(const aiScene* scene, aiNode* node, int parent = 0);
    Mesh processMesh(const aiScene* scene, const aiMesh* aMesh);
    Mesh loadMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    Texture loadMaterialTexture(aiMaterial* mat, aiTextureType type);
    Material processMaterial(const aiScene* scene, const aiMesh* aMesh);
    Transform toTransform(const aiMatrix4x4& m);
};
