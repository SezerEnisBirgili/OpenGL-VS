#include "AssimpImporter.h"

#include <iostream>
#include <algorithm>

AssimpImporter::AssimpImporter(Registry& registry, const std::string& modelPath, int shader) : reg(registry), path(modelPath), shader(shader)
{
    std::replace(path.begin(), path.end(), '\\', '/');

    size_t lastSlash = path.find_last_of('/');
    directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";
}

int AssimpImporter::loadModel(const std::string& rootName, const TransformComponent& transform, int parent)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return NULL_ENTITY;
    }

    return processNode(scene->mRootNode, scene, rootName, transform, parent);
}

int AssimpImporter::processNode(aiNode* node, const aiScene* scene, const std::string& name, const TransformComponent& transform, int parent)
{
    int nodeEntity = EntityBuilder::create(reg, name, transform.position, transform.scale, parent);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
        std::string meshName = name + "_mesh" + std::to_string(i);
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        MaterialComponent material{};

        // unpack mesh data from aMesh
        // does not create mesh
        processMesh(aMesh, scene, material, vertices, indices);

        // create and register mesh in this scope
        int meshId = reg.registerMesh(vertices, indices, meshName);

        EntityBuilder::create(reg, meshName, glm::vec3(0.0f), glm::vec3(1.0f), nodeEntity)
            .mesh(meshId, shader, material);
    }

    // 4. Recursively process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, name, TransformComponent{}, nodeEntity);
    }

    return nodeEntity;
}

void AssimpImporter::processMesh(aiMesh* aMesh, const aiScene* scene, MaterialComponent& outMaterial, std::vector<Vertex> &vertices, std::vector<unsigned int> &indices)
{
    for (unsigned int i = 0; i < aMesh->mNumVertices; i++) {
        Vertex v{};
        v.Position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };
        v.Normal   = { aMesh->mNormals[i].x,  aMesh->mNormals[i].y,  aMesh->mNormals[i].z };
        v.TexCoords = aMesh->mTextureCoords[0]
            ? glm::vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y)
            : glm::vec2(0.0f);
        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
        aiFace face = aMesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // --- textures now go on the material, not the mesh ---
    if (aMesh->mMaterialIndex < scene->mNumMaterials) {
        aiMaterial* material = scene->mMaterials[aMesh->mMaterialIndex];
        outMaterial.diffuseTexture  = loadMaterialTexture(material, aiTextureType_DIFFUSE);
        outMaterial.specularTexture = loadMaterialTexture(material, aiTextureType_SPECULAR);
    }
}

unsigned int AssimpImporter::loadMaterialTexture(aiMaterial* mat, aiTextureType type)
{
    if (mat->GetTextureCount(type) == 0) return 0;

    aiString str;
    mat->GetTexture(type, 0, &str);

    std::string filename = extractTexturePath(str);
    std::string fullPath = directory + "/" + filename;

    return loadTexture(reg, fullPath, 1, true);
}

std::string AssimpImporter::extractTexturePath(const aiString& str)
{
    std::string filename = str.C_Str();
    std::replace(filename.begin(), filename.end(), '\\', '/');
    return filename;
}