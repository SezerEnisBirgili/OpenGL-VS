#include "AssimpImporter.h"

#include <iostream>
#include <algorithm>

#include <iostream>
#include <filesystem>
#include <assimp/Exporter.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>

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
        std::string meshName = name + "_mesh" + std::to_string(meshCounter++);  // globally unique now
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        MaterialComponent material{};

        processMesh(aMesh, scene, material, vertices, indices);

        int meshId = reg.registerMesh(vertices, indices, meshName);

        EntityBuilder::create(reg, meshName, glm::vec3(0.0f), glm::vec3(1.0f), nodeEntity)
            .renderable(meshId, shader, material);
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


// mainly for simple shapes (e.g. cube for voxel engine)
int AssimpImporter::saveSimpleModel(const std::string& rootName, const float* vertices, int vertexCount, int vertexSize, const unsigned int* indices, int indexCount, int indicesPerFace)
{

    // auto create missing directory path
    std::filesystem::path outPath = std::filesystem::absolute(rootName);
    if (outPath.has_parent_path()) {
        std::filesystem::create_directories(outPath.parent_path());
    }
    std::string cleanPath = outPath.generic_string();
    std::cout << "Attempting export to: " << cleanPath << std::endl;

    aiScene* scene = new aiScene();
    scene->mRootNode = new aiNode();

    scene->mMaterials = new aiMaterial * [1];
    scene->mMaterials[0] = new aiMaterial();
    scene->mNumMaterials = 1;

    scene->mMeshes = new aiMesh * [1];
    scene->mMeshes[0] = new aiMesh();
    scene->mNumMeshes = 1;
    scene->mMeshes[0]->mMaterialIndex = 0;

    scene->mRootNode->mMeshes = new unsigned int[1];
    scene->mRootNode->mMeshes[0] = 0;
    scene->mRootNode->mNumMeshes = 1;

    addVerticesToMesh(vertices, vertexCount, vertexSize, scene->mMeshes[0]);
    addIndicesToMesh(indices, indexCount, scene->mMeshes[0], indicesPerFace);

    Assimp::Exporter exporter;
    aiReturn result = exporter.Export(scene, "gltf2", rootName);

    if (result != aiReturn_SUCCESS) {
        std::cerr << "Assimp ErrorString: " << exporter.GetErrorString() << std::endl;
    }

    // 3. Clean up the logger when done
    Assimp::DefaultLogger::kill();
    delete scene;

    return (result == aiReturn_SUCCESS) ? 0 : -1;
}

// indicesPerFace: indices are based on triangles therefore 3 per by default
void AssimpImporter::addIndicesToMesh(const unsigned int* indices, int indexCount, aiMesh* mesh, const int indicesPerFace)
{
    int faceCount = indexCount / indicesPerFace;

    mesh->mNumFaces = faceCount;
    mesh->mFaces = new aiFace[faceCount];

    for (int i = 0; i < faceCount; ++i) {
        int offset = i * indicesPerFace;

        aiFace& face = mesh->mFaces[i];
        face.mNumIndices = indicesPerFace;
        face.mIndices = new unsigned int[indicesPerFace];

        face.mIndices[0] = indices[offset];
        face.mIndices[1] = indices[offset + 1];
        face.mIndices[2] = indices[offset + 2];
    }
}

void AssimpImporter::addVerticesToMesh(const float* vertices, int vertexCount, int vertexSize, aiMesh* mesh)
{
    mesh->mNumVertices = vertexCount;

    mesh->mVertices = new aiVector3D[vertexCount];
    mesh->mNormals = new aiVector3D[vertexCount];
    mesh->mTextureCoords[0] = new aiVector3D[vertexCount];
    mesh->mNumUVComponents[0] = 2;

    for (int i = 0; i < vertexCount; ++i) {
        int offset = i * vertexSize;

        // 5. Populate vertices
        mesh->mVertices[i] = aiVector3D(
            vertices[offset],
            vertices[offset + 1],
            vertices[offset + 2]
        );

        // 6. Populate normals
        mesh->mNormals[i] = aiVector3D(
            vertices[offset + 3],
            vertices[offset + 4],
            vertices[offset + 5]
        );

        // 7. Populate texture UVs
        mesh->mTextureCoords[0][i] = aiVector3D(
            vertices[offset + 6],
            vertices[offset + 7],
            0.0f
        );
    }
}