#include "AssimpImporter.h"

#include <iostream>
#include <algorithm>

AssimpImporter::AssimpImporter(Registry& registry, const std::string& modelPath, Shader* shader)
    : reg(registry), path(modelPath), shader(shader)
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

int AssimpImporter::processNode(aiNode* node, const aiScene* scene, const std::string& name,
    const TransformComponent& transform, int parent)
{
    int nodeEntity = spawnEntity(reg, name, transform, parent);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];

        MaterialComponent material{};
        Mesh mesh = processMesh(aMesh, scene, material);

        int meshId = reg.registerMesh(mesh);

        int meshEntity = spawnEntity(reg, name + "_mesh" + std::to_string(i), TransformComponent{}, nodeEntity);
        addMesh(reg, meshEntity, meshId, shader, material);
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(node->mChildren[i], scene, name, TransformComponent{}, nodeEntity);

    return nodeEntity;
}

Mesh AssimpImporter::processMesh(aiMesh* aMesh, const aiScene* scene, MaterialComponent& outMaterial)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

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

    // --- build the GPU-side Mesh manually; no constructor to call anymore ---
    Mesh mesh;
    mesh.indexCount = (int)indices.size();

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);   // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));

    glEnableVertexAttribArray(1);   // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);   // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);

    mesh.setupInstanceBuffer();   // pre-create instanceVBO so it's ready if this mesh is ever instanced

    return mesh;
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