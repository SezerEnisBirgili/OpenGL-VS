
#include "model.h"
#include "assimp/postprocess.h"
#include <iostream>

void Model::loadModel(std::string path) 
{
    std::replace(path.begin(), path.end(), '\\', '/');

    size_t lastSlash = path.find_last_of('/');
    directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    }

    return processNode(scene, scene->mRootNode);
}

void Model::processNode(const aiScene* scene, aiNode* node, int depth)
{
    hierarchy.push_back(depth);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) 
    {
        aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(scene, aMesh));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(scene, node->mChildren[i], depth++);
    }
}

Mesh Model::processMesh(const aiScene* scene, const aiMesh* aMesh)
{
    Mesh mesh;
    mesh.vertices.reserve(aMesh->mNumVertices);
    mesh.indices.reserve(aMesh->mNumFaces);

    for (unsigned int i = 0; i < aMesh->mNumVertices; i++) {
        Vertex v{};
        v.Position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };
        v.Normal = aMesh->HasNormals() ? glm::vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z) : glm::vec3(0.0f);
        v.TexCoords = aMesh->mTextureCoords[0] ? glm::vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y) : glm::vec2(0.0f);
        mesh.vertices.push_back(v);
    }

    if (aMesh->HasFaces()) 
    {
        for (unsigned int i = 0; i < aMesh->mNumFaces; i++) 
        {
            aiFace face = aMesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                mesh.indices.push_back(face.mIndices[j]);
        }
    }

    if (aMesh->mMaterialIndex < scene->mNumMaterials) {
        aiMaterial* material = scene->mMaterials[aMesh->mMaterialIndex];
        mesh.diffuseTexturePath  = getMaterialTexturePath(material, aiTextureType_DIFFUSE);
        mesh.specularTexturePath = getMaterialTexturePath(material, aiTextureType_SPECULAR);
    }

    return mesh;
}

std::string Model::getMaterialTexturePath(aiMaterial* mat, aiTextureType type) {
    if (mat->GetTextureCount(type) == 0) return 0;

    aiString str;
    if (mat->GetTexture(type, 0, &str) != AI_SUCCESS) return 0;
    std::string filename = str.C_Str();
    std::string fullPath = directory + '/' + filename;

    return fullPath;
}