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

struct RaycastHit3D;
enum class BoxFace3D;

template <typename Visitor>
bool traverseDDA(glm::vec3 start, glm::vec3 front, Visitor&& visit, glm::vec3& hit, int LOOP_LIMIT = 100);

RaycastHit3D intersectRayAABB3D(const glm::vec3& start, const glm::vec3& rayDir, const glm::vec3& min, const glm::vec3& max);


enum class BoxFace3D {
    None,
    Left,   // -X
    Right,  // +X
    Bottom, // -Y
    Top,    // +Y
    Back,   // -Z
    Front   // +Z
};

struct RaycastHit3D {
    bool collided = false;
    float t = -1.0f;
    glm::vec3 point = { 0.0f, 0.0f, 0.0f };
    BoxFace3D face = BoxFace3D::None;
};

class Block {
public:
    bool isSolid;
    int m_material;

    Block(bool solid, int material) : isSolid(solid), m_material(material) {}
};

struct TextureSlot {
    unsigned int texture;
    std::string uniformName;
};

class Material {
private:
    std::vector<TextureSlot> textures;
    float shininess;

public:
    Material(std::vector<TextureSlot> textures, float shininess = 32.0f) : textures(std::move(textures)), shininess(shininess) {}

    void bind(Shader& shader) const {
        for (size_t i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + (GLenum)i);
            glBindTexture(GL_TEXTURE_2D, textures[i].texture);
            shader.setInt(textures[i].uniformName, (int)i);
        }
        shader.setFloat("material.shininess", shininess);
    }
};

class MaterialRegistry {
private:
    std::unordered_map<int, Material> materials;

public:
    void add(int id, Material material) {
        materials.emplace(id, std::move(material));
    }

    const Material& get(int id) const {
        return materials.at(id);
    }
};

class World {

private:

    int boundx, boundy, boundz;

    std::vector<Block> blocks;

    int getIndex(int x, int y, int z) const
    {
        return (x * boundy * boundz) + (y * boundz) + z;
    }

public:

    World(int x, int y, int z) : boundx(x), boundy(y), boundz(z)
    {
        // initialize with empty air blocks
        blocks.resize(x * y * z, Block(false, 0));
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

    // Bounds check to assume 0 to bound coordinates
    bool isWithinBounds(int x, int y, int z) const
    {
        return (x >= 0 && x < boundx && y >= 0 && y < boundy && z >= 0 && z < boundz);
    }

    bool isBlockSolid(int x, int y, int z) const
    {
        if (!isWithinBounds(x, y, z)) return false;

        return blocks[getIndex(x, y, z)].isSolid;
    }

    Block getBlock(int x, int y, int z)
    {
        if (!isWithinBounds(x, y, z))
            return Block(false, 0);

        return blocks[getIndex(x, y, z)];
    }

    void setBlock(int x, int y, int z, bool isSolid, int material)
    {
        if (isWithinBounds(x, y, z))
            blocks[getIndex(x, y, z)] = Block(isSolid, material);
    }

    void platform(int sizeX, int sizeZ)
    {
        for (int x = 0; x < sizeX; x++)
            for (int z = 0; z < sizeZ; z++)
                setBlock(x, 0, z, true, 0); // y=0 layer, texture id 0
    }

    bool placeBlock(const glm::vec3& start, const glm::vec3& front, int newTexture, int maxDistance = 100) {
        glm::vec3 hitBlock;

        bool hitSomething = traverseDDA(start, front, [this](int x, int y, int z) { return isBlockSolid(x, y, z); }, hitBlock, maxDistance);

        int hitX = static_cast<int>(std::floor(hitBlock.x));
        int hitY = static_cast<int>(std::floor(hitBlock.y));
        int hitZ = static_cast<int>(std::floor(hitBlock.z));

        std::cout << "[placeBlock] hitSomething=" << hitSomething
            << " hitBlock=(" << hitBlock.x << "," << hitY << "," << hitZ << ")" << std::endl;

        if (!hitSomething || !isWithinBounds(hitX, hitY, hitZ))
        {
            std::cout << "[placeBlock] failed: no hit or out of bounds" << std::endl;
            return false;
        }

        glm::vec3 min = glm::vec3(hitX, hitY, hitZ);
        glm::vec3 max = glm::vec3(min.x + 1.0f, min.y + 1.0f, min.z + 1.0f);

        RaycastHit3D rayHit = intersectRayAABB3D(start, front, min, max);

        std::cout << "[placeBlock] rayHit.collided=" << rayHit.collided
            << " face=" << (int)rayHit.face << std::endl;

        if (!rayHit.collided) {
            std::cout << "[placeBlock] failed: AABB raycast missed" << std::endl;
            return false;
        }

        switch (rayHit.face) {
        case BoxFace3D::Left:   hitX -= 1; break;
        case BoxFace3D::Right:  hitX += 1; break;
        case BoxFace3D::Bottom: hitY -= 1; break;
        case BoxFace3D::Top:    hitY += 1; break;
        case BoxFace3D::Back:   hitZ -= 1; break;
        case BoxFace3D::Front:  hitZ += 1; break;
        default:
            std::cout << "[placeBlock] failed: face == None" << std::endl;
            return false;
        }

        std::cout << "[placeBlock] target placement=(" << hitX << "," << hitY << "," << hitZ << ")"
            << " inBounds=" << isWithinBounds(hitX, hitY, hitZ)
            << " alreadySolid=" << isBlockSolid(hitX, hitY, hitZ) << std::endl;

        if (isWithinBounds(hitX, hitY, hitZ) && !isBlockSolid(hitX, hitY, hitZ)) {
            setBlock(hitX, hitY, hitZ, true, newTexture);
            std::cout << "[placeBlock] SUCCESS" << std::endl;
            return true;
        }

        std::cout << "[placeBlock] failed: target obstructed or out of bounds";

        return false;
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

        for (int x = 0; x < boundx; ++x) {
            for (int y = 0; y < boundy; ++y) {
                for (int z = 0; z < boundz; ++z) {
                    const Block& b = blocks[getIndex(x, y, z)];
                    outFile << b.isSolid << " " << b.m_material << "\n";
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

        for (int x = 0; x < bx; ++x) {
            for (int y = 0; y < by; ++y) {
                for (int z = 0; z < bz; ++z) {
                    bool solid;
                    int material;
                    inFile >> solid >> material;
                    setBlock(x, y, z, solid, material);
                }
            }
        }

        inFile.close();
        std::cout << "Successfully imported world from: " << sourcePath << std::endl;
        return true;
    }

    void draw(Shader& shader, const MaterialRegistry& materials) const {
        shader.use();

        shader.setVec3("material.color", glm::vec3(1.0f));
        shader.setFloat("material.shininess", 32.0f);
        shader.setFloat("material.emissive", 0.0f);

        std::unordered_map<int, std::vector<glm::vec3>> byTexture;

        for (int x = 0; x < boundx; x++)
            for (int y = 0; y < boundy; y++)
                for (int z = 0; z < boundz; z++) 
                {
                    const Block& b = blocks[getIndex(x, y, z)];
                    if (!b.isSolid) continue;
                    byTexture[b.m_material].emplace_back(x, y, z);
                }

        for (auto& [id, positions] : byTexture) 
        {
            materials.get(id).bind(shader);
            for (const auto& pos : positions) 
            {
                glm::vec3 renderPos = pos + glm::vec3(0.5f); // center offset for -0.5..0.5 mesh
                shader.setMat4("model", glm::translate(glm::mat4(1.0f), renderPos));
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
    }
};


template <typename Visitor>
bool traverseDDA(glm::vec3 start, glm::vec3 front, Visitor&& visit, glm::vec3& hit, int LOOP_LIMIT)
{
    if (front.x == 0 && front.y == 0 && front.z == 0)
        return false;

    front = glm::normalize(front);

    int x = static_cast<int>(std::floor(start.x));
    int y = static_cast<int>(std::floor(start.y));
    int z = static_cast<int>(std::floor(start.z));

    int dir_x = (front.x > 0) - (front.x < 0);
    int dir_y = (front.y > 0) - (front.y < 0);
    int dir_z = (front.z > 0) - (front.z < 0);

    const float BIG = 1e30f;

    // if front is normalized, scaling factor calculation simplies to this
    float S_x = (front.x != 0) ? std::abs(1.0f / front.x) : INFINITY;
    float S_y = (front.y != 0) ? std::abs(1.0f / front.y) : INFINITY;
    float S_z = (front.z != 0) ? std::abs(1.0f / front.z) : INFINITY;

    float D_x = (dir_x < 0) ? (start.x - x) * S_x : (x + 1 - start.x) * S_x;
    float D_y = (dir_y < 0) ? (start.y - y) * S_y : (y + 1 - start.y) * S_y;
    float D_z = (dir_z < 0) ? (start.z - z) * S_z : (z + 1 - start.z) * S_z;

    for (int looped = 0; looped < LOOP_LIMIT; looped++)
    {
        if (visit(x, y, z))
        {
            hit = glm::vec3(x, y, z);
            return true;
        }

        if (D_x >= D_y) 
        {
            if (D_y >= D_z)  { z += dir_z; D_z += S_z; }
            else             { y += dir_y; D_y += S_y; }
        }

        else if (D_x >= D_z) { z += dir_z; D_z += S_z; }
        else                 { x += dir_x; D_x += S_x; }
    }

    return false;
}


RaycastHit3D intersectRayAABB3D(const glm::vec3& start, const glm::vec3& rayDir, const glm::vec3& min, const glm::vec3& max) {

    RaycastHit3D hit;

    float tMinX = (min.x - start.x) / rayDir.x;
    float tMaxX = (max.x - start.x) / rayDir.x;
    BoxFace3D nearFaceX = BoxFace3D::Left;
    if (tMinX > tMaxX) {
        std::swap(tMinX, tMaxX);
        nearFaceX = BoxFace3D::Right;
    }

    float tMinY = (min.y - start.y) / rayDir.y;
    float tMaxY = (max.y - start.y) / rayDir.y;
    BoxFace3D nearFaceY = BoxFace3D::Bottom;
    if (tMinY > tMaxY) {
        std::swap(tMinY, tMaxY);
        nearFaceY = BoxFace3D::Top;
    }

    float tMinZ = (min.z - start.z) / rayDir.z;
    float tMaxZ = (max.z - start.z) / rayDir.z;
    BoxFace3D nearFaceZ = BoxFace3D::Back;
    if (tMinZ > tMaxZ) {
        std::swap(tMinZ, tMaxZ);
        nearFaceZ = BoxFace3D::Front;
    }

    float tNear = std::max({ tMinX, tMinY, tMinZ });
    float tFar = std::min({ tMaxX, tMaxY, tMaxZ });

    if (tNear <= tFar && tFar >= 0.0f) {
        hit.collided = true;
        hit.t = (tNear < 0.0f) ? 0.0f : tNear; // If origin is inside the box, t is 0

        hit.point.x = start.x + rayDir.x * hit.t;
        hit.point.y = start.y + rayDir.y * hit.t;
        hit.point.z = start.z + rayDir.z * hit.t;

        if (tNear == tMinX) {
            hit.face = nearFaceX;
        }
        else if (tNear == tMinY) {
            hit.face = nearFaceY;
        }
        else {
            hit.face = nearFaceZ;
        }
        return hit;
    }

    return hit; // Missed
}


