#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "GameObject.h"

class AssimpImporter 
{
public:
    AssimpImporter(Registry& registry, const std::string& modelPath, Shader* shader) : reg(registry), path(modelPath), shader(shader)
    {
        directory = path.substr(0, path.find_last_of('/'));
    }

    inline int loadModel(const std::string& rootName, const TransformComponent& transform = TransformComponent{}, int parent = 0 /*NULL_ENTITY*/)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return NULL_ENTITY;
        }

        return processNode(scene->mRootNode, scene, rootName, transform, parent);
    }

    int processNode(aiNode* node, const aiScene* scene, const std::string& name, const TransformComponent& transform = TransformComponent{}, int parent = 0 /*NULL_ENTITY*/)
    {
        int nodeEntity = spawnEntity(reg, name, transform, parent);

        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
            Mesh mesh = processMesh(aMesh, scene);

            int meshEntity = spawnEntity(reg, name + "_mesh" + std::to_string(i), TransformComponent{}, nodeEntity);
            addMesh(reg, meshEntity, std::move(mesh), shader);
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene, name, TransformComponent{}, nodeEntity);

        return nodeEntity;
    }

    inline Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex v{};
            v.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            v.Normal =   {  mesh->mNormals[i].x,  mesh->mNormals[i].y,  mesh->mNormals[i].z };

            if(mesh->mTextureCoords[0]) {
                v.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            } else {
               v.TexCoords = glm::vec2(0.0f);
            }
            vertices.push_back(v);
        }

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        unsigned int diffuseTexture = 0;
        unsigned int specularTexture = 0;

        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            diffuseTexture  = loadMaterialTexture(material, aiTextureType_DIFFUSE);
            specularTexture = loadMaterialTexture(material, aiTextureType_SPECULAR);
        }

        return Mesh(vertices, indices, diffuseTexture, specularTexture);
    }

    // 3.1.1 assimp getTexture is broken
    inline unsigned int loadMaterialTexture(aiMaterial* mat, aiTextureType type)
    {
        if (mat->GetTextureCount(type) == 0) return 0;

        aiString str;
        mat->GetTexture(type, 0, &str);

        std::string filename = extractTexturePath(str);
        std::string fullPath = directory + "/" + filename;

        return reg.loadTexture(fullPath, 0, true); 
    }

    std::string extractTexturePath(const aiString& str)
    {
        const char* raw = reinterpret_cast<const char*>(&str);

        for (size_t i = 0; i < sizeof(aiString) - 5; ++i) {
            if (std::isalnum(static_cast<unsigned char>(raw[i]))) {
                const char* start = raw + i;
                const char* end = std::strchr(start, '\0');

                if (end != nullptr) {
                    if (std::memchr(start, '.', end - start) != nullptr) {
                        // 1. Extract filename
                        std::string filename = std::string(start, end);
                        
                        // 2. BREAK IMMEDIATELY so trailing bytes don't overwrite it!
                        return filename; 
                    }
                }
            }
        }

        return str.C_Str();
    }
private:
    Registry& reg;
    std::string path;
    std::string directory;
    std::vector<Mesh> meshes;
    Shader* shader = nullptr;
};