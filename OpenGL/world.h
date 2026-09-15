#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <utility>
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

    Registry* reg = nullptr;

public:
    World() = default;

    World(Registry* registry, int x, int y, int z);

    ~World() = default;

    World(const World&) = delete;
    World& operator=(const World&) = delete;

    World(World&& other) noexcept {
        boundx = other.boundx;
        boundy = other.boundy;
        boundz = other.boundz;
        blocks = std::move(other.blocks);
        outlineColor = other.outlineColor;
        reg = other.reg;

        other.boundx = 0;
        other.boundy = 0;
        other.boundz = 0;
        other.reg = nullptr;
    }

    World& operator=(World&& other) noexcept {
        if (this != &other) {
            boundx = other.boundx;
            boundy = other.boundy;
            boundz = other.boundz;
            blocks = std::move(other.blocks);
            outlineColor = other.outlineColor;
            reg = other.reg;

            other.boundx = 0;
            other.boundy = 0;
            other.boundz = 0;
            other.reg = nullptr;
        }
        return *this;
    }

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
    void removeBlock(const glm::vec3& v);
    void createPlatform(int e, int sizeX, int sizeZ);

    bool exportWorldToPath(const std::string& destinationPath) const;
    bool importWorldFromPath(Registry& reg, const std::string& sourcePath);

    void collectRenderItems(std::vector<RenderItem>& out, const glm::mat4& parentTransform) const override;
    void collectInstancedRenderItems(std::vector<InstancedRenderItem>& out, const glm::mat4& parentTransform) const override;
};