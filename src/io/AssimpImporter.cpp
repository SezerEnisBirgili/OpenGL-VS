#include "AssimpImporter.h"

#include <iostream>
#include <algorithm>

#include <iostream>
#include <filesystem>
#include <assimp/Exporter.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/scene.h>

class Model 
{
    public:
        Model(char *path)
        {
            loadModel(path);
        }
        void Draw(Shader &shader);	
    private:
        // model data
        std::vector<Mesh> meshes;
        std::string directory;

        void loadModel(std::string path) 
        {
            std::replace(path.begin(), path.end(), '\\', '/');

            size_t lastSlash = path.find_last_of('/');
            directory = (lastSlash != std::string::npos) ? path.substr(0, lastSlash) : ".";

            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            }

            return processNode(scene->mRootNode);
        }
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};

int AssimpImporter::processNode(aiNode* node, const aiScene* scene, const std::string& name, const TransformComponent& transform, int parent)
{
    int nodeEntity = EntityBuilder::create(reg, name, transform.position, transform.scale, parent);

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {

        aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
        std::string meshName = name + "_mesh" + std::to_string(meshCounter++);
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

        // Populate vertices
        mesh->mVertices[i] = aiVector3D(
            vertices[offset],
            vertices[offset + 1],
            vertices[offset + 2]
        );

        // Populate normals
        mesh->mNormals[i] = aiVector3D(
            vertices[offset + 3],
            vertices[offset + 4],
            vertices[offset + 5]
        );

        // Populate texture UVs
        mesh->mTextureCoords[0][i] = aiVector3D(
            vertices[offset + 6],
            vertices[offset + 7],
            0.0f
        );
    }
}

std::vector<float> AssimpImporter::meshToVertices(const aiMesh* mesh, const int vertexSize) 
{
    if (!mesh || mesh->mNumVertices == 0) { return {}; }

    std::vector<float> vertices(mesh->mNumVertices * vertexSize);

    bool hasNormals = mesh->HasNormals();
    bool hasUVs = mesh->HasTextureCoords(0);

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) 
    {
        int offset = i * vertexSize;

        // Extract vertex positions (3 floats)
        vertices[offset + 0] = mesh->mVertices[i].x;
        vertices[offset + 1] = mesh->mVertices[i].y;
        vertices[offset + 2] = mesh->mVertices[i].z;

        // Extract normals (3 floats, fallback to 0.0f if missing)
        if (hasNormals) {
            vertices[offset + 3] = mesh->mNormals[i].x;
            vertices[offset + 4] = mesh->mNormals[i].y;
            vertices[offset + 5] = mesh->mNormals[i].z;
        } else {
            vertices[offset + 3] = 0.0f;
            vertices[offset + 4] = 0.0f;
            vertices[offset + 5] = 0.0f;
        }

        // Extract texture UVs (2 floats, fallback to 0.0f if missing)
        if (hasUVs) {
            vertices[offset + 6] = mesh->mTextureCoords[0][i].x;
            vertices[offset + 7] = mesh->mTextureCoords[0][i].y;
        } else {
            vertices[offset + 6] = 0.0f;
            vertices[offset + 7] = 0.0f;
        }
    }

    return vertices;
}

std::vector<unsigned int> AssimpImporter::meshToIndices(const aiMesh* mesh, const int indicesPerFace) {
    if (!mesh || mesh->mNumFaces == 0) { return {};}

    std::vector<unsigned int> indices;
    // Reserve space assuming triangles (3 indices per face)
    indices.reserve(mesh->mNumFaces * indicesPerFace);

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) 
    {
        const aiFace& face = mesh->mFaces[i];
        
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return indices;
}
