#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "GameObject.h"
#include "mesh.h"

class AssimpImporter {
public:
    AssimpImporter(Registry& registry, const std::string& modelPath, Shader* shader);

    int loadModel(const std::string& rootName, const TransformComponent& transform = TransformComponent{}, int parent = NULL_ENTITY);

private:
    int processNode(aiNode* node, const aiScene* scene, const std::string& name, const TransformComponent& transform = TransformComponent{}, int parent = NULL_ENTITY);

    Mesh processMesh(aiMesh* mesh, const aiScene* scene, MaterialComponent& outMaterial);
    unsigned int loadMaterialTexture(aiMaterial* mat, aiTextureType type);
    std::string extractTexturePath(const aiString& str);

    Registry& reg;
    std::string path;
    std::string directory;
    Shader* shader = nullptr;
};