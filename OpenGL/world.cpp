#include "world.h"
#include "GameObject.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

World::World(Registry& registry, int x, int y, int z) : reg(registry), boundx(x), boundy(y), boundz(z) {
    blocks.resize(x * y * z, 0);
}

int World::getIndex(const glm::vec3& v) const {
    return (int)v.x * boundy * boundz + (int)v.y * boundz + (int)v.z;
}

bool World::isBlockSolid(const glm::vec3& v) const {
    if (!isWithinBounds(v)) return false;

    int e = blocks[getIndex(v)];
    if (e == NULL_ENTITY) return false;   // empty space is never solid — skip the lookup entirely

    auto it = reg.materials.find(e);
    if (it == reg.materials.end()) return false;   // defensive: unregistered id, treat as non-solid

    return !it->second.isAir;
}

int World::getBlock(const glm::vec3& v) const {
    if (!isWithinBounds(v)) return NULL_ENTITY;
    return blocks[getIndex(v)];
}

void World::setBlock(int e, const glm::vec3& v) {
    if (isWithinBounds(v))  {
        blocks[getIndex(v)] = e;

        // danger: std out of range
        if (reg.materials.at(e).isTransparent) {
            transparent[e].push_back(v);
        } else {
            opaque[e].push_back(v);
        }
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
            std::cerr << "Failed to create directory path: " << e.what() << std::endl;
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

    outFile.close();
    std::cout << "Successfully exported world to: " << destinationPath << std::endl;
    return true;
}

bool World::importWorldFromPath(Registry& reg, const std::string& sourcePath) {
    if (!std::filesystem::exists(sourcePath)) {
        std::cerr << "Error: File does not exist at path: " << sourcePath << std::endl;
        return false;
    }

    std::ifstream inFile(sourcePath);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for importing: " << sourcePath << std::endl;
        return false;
    }

    int bx, by, bz;
    if (!(inFile >> bx >> by >> bz)) {
        std::cerr << "Failed to read dimensions from: " << sourcePath << std::endl;
        return false;
    }

    boundx = bx;
    boundy = by;
    boundz = bz;
    blocks.assign(boundx * boundy * boundz, 0);
    opaque.clear();
    transparent.clear();

    std::string line;
    std::getline(inFile, line);

    for (int x = 0; x < boundx; ++x) {
        for (int y = 0; y < boundy; ++y) {
            for (int z = 0; z < boundz; ++z) {
                if (!std::getline(inFile, line)) break;

                std::stringstream ss(line);
                int e = 0;
                ss >> e;

                if (e != NULL_ENTITY) {
                    setBlock(e, glm::vec3(x, y, z));   // keeps blocks[] and opaque/transparent in sync
                }
            }
        }
    }

    inFile.close();

    std::cout << "Successfully imported world from: " << sourcePath << std::endl;
    return true;
}


// Opaque only
void World::collectInstancedRenderItems(std::vector<InstancedRenderItem>& out, const glm::mat4& parentTransform) const {
    for (auto& [e, positions] : opaque) {
        std::vector<glm::vec3> centered;
        centered.reserve(positions.size());
        for (const auto& pos : positions)
            centered.push_back(blockCenter(pos));

        out.push_back({ e, std::move(centered) });
    }
}

// transparent only
void World::collectRenderItems(std::vector<RenderItem>& out, const glm::mat4& parentTransform) const {
    for (auto& [e, positions] : transparent) {
        for (auto& pos : positions) {
            glm::vec3 worldPos = blockCenter(glm::vec3(parentTransform * glm::vec4(pos, 1.0f)));
            out.push_back({ e, worldPos });
        }
    }
}