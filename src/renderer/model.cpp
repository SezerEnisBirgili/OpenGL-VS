#include "model.h"

#include "renderer/model.h"
#include "renderer/texture.h"
#include "renderer/transform.h"

#include "assimp/postprocess.h"

void ModelLoader::loadModel(std::string path)
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

Transform ModelLoader::toTransform(const aiMatrix4x4& m)
{
    aiVector3D pos, scl;
    aiQuaternion rot;
    m.Decompose(scl, rot, pos);

    Transform t;
    t.position = { pos.x, pos.y, pos.z };
    t.rotation = glm::quat(rot.w, rot.x, rot.y, rot.z);  // glm is w-first
    t.scale = { scl.x, scl.y, scl.z };
    return t;
}

void ModelLoader::processNode(const aiScene* scene, aiNode* node, int parent)
{
    const int self = model.nodes.size();
    model.nodes.push_back({ Mesh{}, Material{}, toTransform(node->mTransformation), parent });

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned meshIndex = node->mMeshes[i];
        std::string key = path + "#" + std::to_string(meshIndex);
        if (registry.contains())
        const aiMesh* aMesh = scene->mMeshes[meshIndex];

        Mesh mesh = processMesh(scene, aMesh);
        Material material = processMaterial(scene, aMesh);
        
        int meshId = registry.add(mesh);

        if (i == 0) {
            model.nodes[self].mesh = std::move(mesh);
            model.nodes[self].material = material;
        }
        else {
            model.nodes.push_back({ std::move(mesh), material, Transform{}, self });
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
        processNode(scene, node->mChildren[i], self);
}

Mesh ModelLoader::processMesh(const aiScene* scene, const aiMesh* aMesh)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(aMesh->mNumVertices);
    indices.reserve(aMesh->mNumFaces * 3);

    for (unsigned int i = 0; i < aMesh->mNumVertices; i++) {
        Vertex v{};
        v.Position = { aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z };
        v.Normal = aMesh->HasNormals()
            ? glm::vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z)
            : glm::vec3(0.0f);
        v.TexCoords = aMesh->mTextureCoords[0]
            ? glm::vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y)
            : glm::vec2(0.0f);
        vertices.push_back(v);
    }

    for (unsigned int i = 0; i < aMesh->mNumFaces; i++) {
        const aiFace& face = aMesh->mFaces[i];
        indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
    }

    return loadMesh(vertices, indices);
}

Mesh ModelLoader::loadMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) 
{
    Mesh mesh;

    mesh.indexCount = indices.size();

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);

    return mesh;
}

Material ModelLoader::processMaterial(const aiScene* scene, const aiMesh* aMesh)
{
    Material material;

    if (aMesh->mMaterialIndex < scene->mNumMaterials)
    {
        aiMaterial* aMat = scene->mMaterials[aMesh->mMaterialIndex];
        material.diffuse = loadMaterialTexture(aMat, aiTextureType_DIFFUSE);
        material.specular = loadMaterialTexture(aMat, aiTextureType_SPECULAR);
    }
    return material;
}

Texture ModelLoader::loadMaterialTexture(aiMaterial* mat, aiTextureType type)
{
    if (mat->GetTextureCount(type) == 0) return Texture{};

    aiString str;
    mat->GetTexture(type, 0, &str);
    const std::string filename = str.C_Str();

    if (auto it = loadedTextures.find(filename); it != loadedTextures.end())
        return it->second;

    Texture texture = loadTextureFromFile(model.directory + '/' + filename);
    texture.type = (type == aiTextureType_SPECULAR) ? TextureType::SPECULAR : TextureType::DIFFUSE;

    loadedTextures[filename] = texture;
    return texture;
}