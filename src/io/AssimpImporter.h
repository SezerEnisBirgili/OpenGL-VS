#pragma once

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <vector>

#include "entities/GameObject.h"

class AssimpImporter {
public:
    AssimpImporter(Registry& registry, const std::string& modelPath, int shader);

    int loadModel(const std::string& rootName, const TransformComponent& transform = TransformComponent{}, int parent = NULL_ENTITY);
    int saveSimpleModel(const std::string& rootName, const float* vertices, int vertexCount, int vertexSize, const unsigned int* indices, int indexCount, const int indicesPerFace);

private:
    int processNode(aiNode* node, const aiScene* scene, const std::string& name, const TransformComponent& transform = TransformComponent{}, int parent = NULL_ENTITY);

    Mesh processMesh(const aiScene* scene, aiMesh* aMesh)
    unsigned int loadMaterialTexture(aiMaterial* mat, aiTextureType type);
    std::string extractTexturePath(const aiString& str);
    void addIndicesToMesh(const unsigned int* indices, int indexCount, aiMesh* mesh, const int indicesPerFace);
    void addVerticesToMesh(const float* vertices, int vertexCount, int vertexSize, aiMesh* mesh);

    Registry& reg;
    std::string path;
    std::string directory;
    int shader = 0;
    int meshCounter = 0;
};