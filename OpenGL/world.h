#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "IRenderable.h"
#include "vertex.h"

inline glm::vec3 blockCenter(const glm::vec3& blockPos) {
    return blockPos + glm::vec3(0.5f);
}

class Shader;
class Registry;

class World : public IRenderable {
private:
    int boundx = 0, boundy = 0, boundz = 0; 
    std::vector<int> blocks;
    glm::vec3 outlineColor = glm::vec3(1.0f);

public:
    std::unordered_map<int, std::vector<glm::vec3>> opaque;
    std::unordered_map<int, std::vector<glm::vec3>> transparent;

    World(int x, int y, int z);

    glm::vec3 getOutlineColor() const { return outlineColor; }
    void setOutlineColor(const glm::vec3& color) { outlineColor = color; }

    int getBoundX() const { return boundx; }
    int getBoundY() const { return boundy; }
    int getBoundZ() const { return boundz; }

    int getIndex(const glm::vec3& v) const;
    bool isWithinBounds(const glm::vec3& v) const { return (v.x >= 0 && v.x < boundx && v.y >= 0 && v.y < boundy && v.z >= 0 && v.z < boundz); }

    bool isBlockSolid(const glm::vec3& v) const;

    int getBlock(const glm::vec3& v) const;
    void setBlock(int e, const glm::vec3& v);
    void createPlatform(int e, int sizeX, int sizeZ);

    bool exportWorldToPath(const std::string& destinationPath) const;
    bool importWorldFromPath(Registry& reg, const std::string& sourcePath);

    void collectRenderItems(std::vector<RenderItem>& out, const glm::mat4& parentTransform) const override;
    void collectInstancedRenderItems(std::vector<InstancedRenderItem>& out, const glm::mat4& parentTransform) const override;
};