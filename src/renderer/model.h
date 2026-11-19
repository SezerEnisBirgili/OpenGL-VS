#include <vector>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include "renderer/mesh.h"

struct Model 
{
public:
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<unsigned int> hierarchy;

    Model(const std::string& path) {
        loadModel(path);
    }

private:
    void loadModel(std::string path);
    void processNode(const aiScene* scene, aiNode* node, int depth = 0);
    Mesh processMesh(const aiScene* scene, const aiMesh* aMesh);
    std::string getMaterialTexturePath(aiMaterial* mat, aiTextureType type);
};