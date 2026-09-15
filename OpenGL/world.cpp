#include "world.h"
#include "GameObject.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

World::World(Registry* registry, int x, int y, int z) : reg(registry), boundx(x), boundy(y), boundz(z) {
    blocks.resize(x * y * z, 0);
}

int World::getIndex(const glm::vec3& v) const {
    return (int)v.x * boundy * boundz + (int)v.y * boundz + (int)v.z;
}

bool World::isBlockSolid(const glm::vec3& v) const {
    if (!isWithinBounds(v)) return false;

    int e = blocks[getIndex(v)];
    if (e == NULL_ENTITY) return false;

    auto it = reg->materials.find(e);
    if (it == reg->materials.end()) {
        std::cerr << "[World::isBlockSolid] Error: Entity " << e << " at block position (" << v.x << ", " << v.y << ", " << v.z << ") has no MaterialComponent!\n";
        return false;
    }

    return !it->second.isAir;
}

int World::getBlock(const glm::vec3& v) const {
    if (!isWithinBounds(v)) return NULL_ENTITY;
    return blocks[getIndex(v)];
}

void World::setBlock(int e, const glm::vec3& v) {
    if (isWithinBounds(v))  {
        blocks[getIndex(v)] = e;
    }
}

void World::removeBlock(const glm::vec3& v) {
    if (isWithinBounds(v)) {
        setBlock(NULL_ENTITY, v);
    }
}

void World::createPlatform(int e, int sizeX, int sizeZ) {
    glm::vec3 v(0.0f);
    for (int x = 0; x < sizeX; x++) {
        for (int z = 0; z < sizeZ; z++) {
            v.x = (float)x;
            v.z = (float)z;
            setBlock(e, v);
        }
    }
}

bool World::exportWorldToPath(const std::string& destinationPath) const {

    std::filesystem::path p(destinationPath);
    std::filesystem::path dir = p.parent_path();

    if (!dir.empty() && !std::filesystem::exists(dir)) {
        try {
            std::filesystem::create_directories(dir);
        }
        catch (const std::filesystem::filesystem_error& e) {
            std::cout << "Failed to create directory path: " << e.what() << std::endl;
            return false;
        }
    }

    std::ofstream outFile(destinationPath);
    if (!outFile.is_open()) {
        std::cerr << "Failed to write to file: " << destinationPath << std::endl;
        return false;
    }

    outFile << boundx << " " << boundy << " " << boundz << "\n";

    for (int x = 0; x < boundx; ++x) {
        for (int y = 0; y < boundy; ++y) {
            for (int z = 0; z < boundz; ++z) {
                int e = blocks[getIndex(glm::vec3(x, y, z))];
                outFile << e << "\n";
            }
        }
    }

    outFile.flush();
    if (!outFile) {
        std::cout << "Error occurred while writing to: " << destinationPath << std::endl;
        return false;
    }

    outFile.close();
    std::cout << "Successfully exported world to: " << destinationPath << std::endl;
    return true;
}

bool World::importWorldFromPath(Registry& reg, const std::string& sourcePath) {
    if (!std::filesystem::exists(sourcePath)) {
        std::cout << "Error: File does not exist at path: " << sourcePath << std::endl;
        return false;
    }

    std::ifstream inFile(sourcePath);
    if (!inFile.is_open()) {
        std::cout << "Failed to open file for importing: " << sourcePath << std::endl;
        return false;
    }

    int bx, by, bz;
    if (!(inFile >> bx >> by >> bz)) {
        std::cout << "Failed to read dimensions from: " << sourcePath << std::endl;
        return false;
    }

    this->reg = &reg;

    int newBoundX = bx, newBoundY = by, newBoundZ = bz;
    std::vector<int> newBlocks(newBoundX * newBoundY * newBoundZ, 0);
    std::unordered_map<int, std::vector<glm::vec3>> newOpaque;
    std::unordered_map<int, std::vector<glm::vec3>> newTransparent;

    std::string line;
    std::getline(inFile, line);

    bool truncated = false;

    for (int x = 0; x < newBoundX && !truncated; ++x) {
        for (int y = 0; y < newBoundY && !truncated; ++y) {
            for (int z = 0; z < newBoundZ; ++z) {
                if (!std::getline(inFile, line)) {
                    std::cout << "Unexpected end of file while importing: " << sourcePath << " (missing block data at " << x << ", " << y << ", " << z << ")" << std::endl;
                    truncated = true;
                    break;
                }

                std::stringstream ss(line);
                int e = 0;
                if (!(ss >> e)) {
                    std::cout << "Malformed block entry in: " << sourcePath << " at (" << x << ", " << y << ", " << z << ")" << std::endl;
                    truncated = true;
                    break;
                }

                if (e == NULL_ENTITY) continue;

                if (!reg.materials.count(e)) {
                    std::cout << "World file references unknown entity " << e << " at (" << x << ", " << y << ", " << z << ") — skipping block." << std::endl;
                    continue;
                }

                int idx = x * newBoundY * newBoundZ + y * newBoundZ + z;
                newBlocks[idx] = e;

                if (reg.getMaterial(e).isTransparent)
                    newTransparent[e].push_back(glm::vec3(x, y, z));
                else
                    newOpaque[e].push_back(glm::vec3(x, y, z));
            }
        }
    }

    if (truncated) {
        std::cout << "Import failed: " << sourcePath << " is corrupted or truncated." << std::endl;
        return false;
    }

    boundx = newBoundX;
    boundy = newBoundY;
    boundz = newBoundZ;
    blocks = std::move(newBlocks);

    inFile.close();

    std::cout << "Successfully imported world from: " << sourcePath << std::endl;
    return true;
}


// Opaque only
void World::collectInstancedRenderItems(std::vector<InstancedRenderItem>& out, const glm::mat4& parentTransform) const {
    std::unordered_map<int, std::vector<glm::vec3>> opaqueGrouped;

    for (int x = 0; x < boundx; ++x) {
        for (int y = 0; y < boundy; ++y) {
            for (int z = 0; z < boundz; ++z) {
                glm::vec3 pos(x, y, z);
                int e = blocks[getIndex(pos)];

                if (e == NULL_ENTITY) continue;

                if (reg && reg->materials.count(e)) {
                    if (!reg->getMaterial(e).isTransparent) {
                        opaqueGrouped[e].push_back(pos);
                    }
                }
            }
        }
    }

    for (auto& [e, positions] : opaqueGrouped) {
        std::vector<glm::vec3> transformedPositions;
        transformedPositions.reserve(positions.size());

        for (const auto& pos : positions) {
            glm::vec3 worldPos = glm::vec3(parentTransform * glm::vec4(blockCenter(pos), 1.0f));
            transformedPositions.push_back(worldPos);
        }

        out.push_back({ e, std::move(transformedPositions) });
    }
}

// Transparent only
void World::collectRenderItems(std::vector<RenderItem>& out, const glm::mat4& parentTransform) const {
    for (int x = 0; x < boundx; ++x) {
        for (int y = 0; y < boundy; ++y) {
            for (int z = 0; z < boundz; ++z) {
                glm::vec3 pos(x, y, z);
                int e = blocks[getIndex(pos)];

                if (e == NULL_ENTITY) continue;

                if (reg && reg->materials.count(e)) {
                    if (reg->getMaterial(e).isTransparent) {
                        glm::vec3 worldPos = glm::vec3(parentTransform * glm::vec4(blockCenter(pos), 1.0f));
                        glm::mat4 model(parentTransform);
                        model[3] = glm::vec4(worldPos, 1.0f);
                        out.push_back({ e, model });
                    }
                }
            }
        }
    }
}