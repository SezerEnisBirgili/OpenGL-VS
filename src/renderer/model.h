#include <vector>

#include "assimp/scene.h"

#include "renderer/transform.h"
#include "renderer/material.h"
#include "renderer/mesh.h"
#include "renderer/registry.h"

struct ModelNode 
{
    Mesh mesh;
    Material material;
    Transform transform;
    int parent;
};

struct Model
{
    std::string directory;
    std::vector<ModelNode> nodes;
};

struct ModelLoader
{
public:
    Model model;
    std::string path;
    std::unordered_map<std::string, Texture> loadedTextures;

    ModelLoader(Registry& reg, const std::string& path): registry(reg) {
        loadModel(path);
    }

private:
    Registry& registry;

    void loadModel(std::string path);
    void processNode(const aiScene* scene, aiNode* node, int parent = 0);
    Mesh processMesh(const aiScene* scene, const aiMesh* aMesh);
    unsigned int loadMaterialTexture(aiMaterial* mat, aiTextureType type);
};
