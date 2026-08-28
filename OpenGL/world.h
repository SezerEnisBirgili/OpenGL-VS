#pragma once
#pragma warning(push)
#pragma warning(disable: 4244)

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <filesystem>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "vertex.h"

class Block {
public:
    bool isSelected = false;
    bool isSolid;
    int d_texture;
    int s_texture;

    Block(bool solid, int diffuseTexture, int specularTexture) : isSolid(solid), d_texture(diffuseTexture), s_texture(specularTexture) {}
};

class World {
private:
    int boundx, boundy, boundz;

    std::vector<Block> blocks;
    Block platformBlock;

    unsigned int cubeVAO = 0, cubeVBO = 0, cubeEBO = 0;
    unsigned int indexCount = 0;

    unsigned int instanceVBO = 0;
    unsigned int activeBlockCount = 0;

    // world is made up from only one type of block
    glm::vec3 color = glm::vec3(1.0f);
    int shininess = 32.0f;
    int emissive = 0.0f;

    // when player points to object
    glm::vec3 outlineColor = glm::vec3(1.0f);

void setupCubeMesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& indices)
    {
        indexCount = indices.size();

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glGenBuffers(1, &cubeEBO);

        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
    }

public:
    World(int x, int y, int z, const Block& block=Block(false, 0, 0)) : boundx(x), boundy(y), boundz(z), platformBlock(block)
    {
        blocks.resize(x * y * z, block);
        platformBlock = Block(true, block.d_texture,block.s_texture);
    }

    glm::vec3 getColor() const { return color; }
    glm::vec3 getOutlineColor() const { return outlineColor; }
    int getShininess() const { return shininess; }
    int getEmissive() const { return emissive; }

    void setOutlineColor(const glm::vec3& color) { outlineColor = color; }

    void initMesh(const std::vector<Vertex>& cubeVerts, const std::vector<unsigned int>& cubeIndices)
    {
        setupCubeMesh(cubeVerts, cubeIndices);

        // Setup Instance VBO for block positions
        glGenBuffers(1, &instanceVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glVertexAttribDivisor(3, 1); // Advance once per instance, not per vertex

        glBindVertexArray(0);
    }

    void updateInstanceBuffer() 
    {
        std::vector<glm::vec3> positions;
        glm::vec3 pos;

        for (int x = 0; x < boundx; ++x) {
            for (int y = 0; y < boundy; ++y) {
                for (int z = 0; z < boundz; ++z) {
                    pos = glm::vec3(x, y, z);
                    if (isBlockSolid(pos)) {
                        positions.push_back(pos + glm::vec3(0.5f)); // <-- offset only for render
                    }
                }
            }
        }

        activeBlockCount = positions.size();
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, activeBlockCount * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Instanced 
    void draw() const 
    {
        if (activeBlockCount == 0 || cubeVAO == 0) return;

        glBindVertexArray(cubeVAO);
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, activeBlockCount);
        glBindVertexArray(0);
    }

    void drawSingleCube() const
    {
        if (cubeVAO == 0) return;

        glBindVertexArray(cubeVAO);
        glDisableVertexAttribArray(3);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glEnableVertexAttribArray(3);
        glBindVertexArray(0);
    }

    int getBlockDiffuseTexture() const {
        return platformBlock.d_texture;
    }

    int getBlockSpecularTexture() const {
        return platformBlock.s_texture;
    }
    
    int getBoundX() const {
        return boundx;
    }

    int getBoundY() const {
        return boundy;
    }

    int getBoundZ() const {
        return boundz;
    }

    int getIndex(const glm::vec3& v) const
    {
        return (v.x * boundy * boundz) + (v.y * boundz) + v.z;
    }

    // Bounds check to assume 0 to bound coordinates
    bool isWithinBounds(const glm::vec3& v) const
    {
        return (v.x >= 0 && v.x < boundx && v.y >= 0 && v.y < boundy && v.z >= 0 && v.z < boundz);
    }

    bool isBlockSolid(const glm::vec3& v) const
    {
        if (!isWithinBounds(v)) return false;

        return blocks[getIndex(v)].isSolid;
    }

    Block* getBlock(const glm::vec3& v)
    {
        if (!isWithinBounds(v))
            return nullptr;

        return &blocks[getIndex(v)];
    }

    void setBlock(const glm::vec3& v)
    {
        if (isWithinBounds(v))
            blocks[getIndex(v)] = platformBlock;
    }

    void platform(int sizeX, int sizeZ)
    {
        glm::vec3 v(0);
        for (int x = 0; x < sizeX; x++) 
        {
            for (int z = 0; z < sizeZ; z++) 
            {
                v.x = x;
                v.z = z;
                setBlock(v); // y=0 layer, texture id 0
            }
        }
    }

    bool exportWorldToPath(const std::string& destinationPath) const {

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

        glm::vec3 index = glm::vec3(0);
        for (int x = 0; x < boundx; ++x) {
            for (int y = 0; y < boundy; ++y) {
                for (int z = 0; z < boundz; ++z) {
                    index.x = x;
                    index.y = y;
                    index.z = z;
                    const Block& b = blocks[getIndex(index)];
                    outFile << b.isSolid << " " << b.d_texture << "\n";
                }
            }
        }

        outFile.close();
        std::cout << "Successfully exported world to: " << destinationPath << std::endl;
        return true;
    }

    bool importWorldFromPath(const std::string& sourcePath) {
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
        inFile >> bx >> by >> bz;
        
        glm::vec3 v = glm::vec3(0);
        for (int x = 0; x < bx; ++x) {
            for (int y = 0; y < by; ++y) {
                for (int z = 0; z < bz; ++z) {
                    bool solid;
                    int texture;

                    inFile >> solid >> texture;

                    v.x = x;
                    v.y = y;
                    v.z = z;
                    setBlock(v);
                }
            }
        }

        inFile.close();
        std::cout << "Successfully imported world from: " << sourcePath << std::endl;
        return true;
    }
};


