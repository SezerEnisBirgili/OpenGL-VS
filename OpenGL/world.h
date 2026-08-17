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

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
};

class Block {
public:
    bool isSolid;
    int texture;

    Block(bool solid, int tex) : isSolid(solid), texture(tex) {}
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

    void setBlock(int x, int y, int z, bool isSolid, int texture)
    {
        if (isWithinBounds(x, y, z))
            blocks[getIndex(x, y, z)] = Block(isSolid, texture);
    }
};


template <typename Visitor>
bool traverseDDA(Vec3 start, Vec3 front, Visitor&& visit, Vec3& hit, int LOOP_LIMIT = 100)
{
    if (front.x == 0 && front.y == 0 && front.z == 0)
        return false;

    int x = (int)std::floor(start.x);
    int y = (int)std::floor(start.y);
    int z = (int)std::floor(start.z);

    int dir_x = (front.x > 0) - (front.x < 0);
    int dir_y = (front.y > 0) - (front.y < 0);
    int dir_z = (front.z > 0) - (front.z < 0);

    const float BIG = 1e30f;
    float S_x = (front.x != 0) ? std::sqrt(1 + (front.y / front.x) * (front.y / front.x) + (front.z / front.x) * (front.z / front.x)) : BIG;
    float S_y = (front.y != 0) ? std::sqrt(1 + (front.x / front.y) * (front.x / front.y) + (front.z / front.y) * (front.z / front.y)) : BIG;
    float S_z = (front.z != 0) ? std::sqrt(1 + (front.x / front.z) * (front.x / front.z) + (front.y / front.z) * (front.y / front.z)) : BIG;

    float D_x = (dir_x < 0) ? (start.x - x) * S_x : (x + 1 - start.x) * S_x;
    float D_y = (dir_y < 0) ? (start.y - y) * S_y : (y + 1 - start.y) * S_y;
    float D_z = (dir_z < 0) ? (start.z - z) * S_z : (z + 1 - start.z) * S_z;

    for (int looped = 0; looped < LOOP_LIMIT; looped++)
    {
        if (visit(x, y, z))
        {
            hit = Vec3(x, y, z);
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

struct AABB3D {
    Vec3 min;
    Vec3 max;
};

// Enum to identify the hit face of the 3D box
enum class BoxFace3D {
    None,
    Left,   // -X
    Right,  // +X
    Bottom, // -Y
    Top,    // +Y
    Back,   // -Z
    Front   // +Z
};

// Struct to hold our complete 3D collision results
struct RaycastHit3D {
    bool collided = false;
    float t = -1.0f;
    Vec3 point = { 0.0f, 0.0f, 0.0f };
    BoxFace3D face = BoxFace3D::None;
};

RaycastHit3D intersectRayAABB3D(const Vec3& start, const Vec3& rayDir, const AABB3D& box) {

    RaycastHit3D hit;

    float tMinX = (box.min.x - start.x) / rayDir.x;
    float tMaxX = (box.max.x - start.x) / rayDir.x;
    BoxFace3D nearFaceX = BoxFace3D::Left;
    if (tMinX > tMaxX) {
        std::swap(tMinX, tMaxX);
        nearFaceX = BoxFace3D::Right;
    }

    float tMinY = (box.min.y - start.y) / rayDir.y;
    float tMaxY = (box.max.y - start.y) / rayDir.y;
    BoxFace3D nearFaceY = BoxFace3D::Bottom;
    if (tMinY > tMaxY) {
        std::swap(tMinY, tMaxY);
        nearFaceY = BoxFace3D::Top;
    }

    float tMinZ = (box.min.z - start.z) / rayDir.z;
    float tMaxZ = (box.max.z - start.z) / rayDir.z;
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

bool placeBlock(const Vec3& start, const Vec3& front, World& world, int newTexture, int maxDistance = 100) {
    Vec3 hitBlock;

    int hitX = static_cast<int>(std::floor(hitBlock.x));
    int hitY = static_cast<int>(std::floor(hitBlock.y));
    int hitZ = static_cast<int>(std::floor(hitBlock.z));

    bool hitSomething = traverseDDA(start, front, [&world](int x, int y, int z) { return world.isBlockSolid(x, y, z); }, hitBlock, maxDistance);

    std::cout << "[placeBlock] hitSomething=" << hitSomething
        << " hitBlock=(" << hitBlock.x << "," << hitY << "," << hitZ << ")" << std::endl;

    if (!hitSomething || !world.isWithinBounds(hitX, hitY, hitZ))
    {
        std::cout << "[placeBlock] failed: no hit or out of bounds" << std::endl;
        return false;
    }

    AABB3D blockAABB;
    blockAABB.min = Vec3(hitX, hitY, hitZ);
    blockAABB.max = Vec3(blockAABB.min.x + 1.0f, blockAABB.min.y + 1.0f, blockAABB.min.z + 1.0f);

    RaycastHit3D rayHit = intersectRayAABB3D(start, front, blockAABB);

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
        << " inBounds=" << world.isWithinBounds(hitX, hitY, hitZ)
        << " alreadySolid=" << world.isBlockSolid(hitX, hitY, hitZ) << std::endl;

    if (world.isWithinBounds(hitX, hitY, hitZ) && !world.isBlockSolid(hitX, hitY, hitZ)) {
        world.setBlock(hitX, hitY, hitZ, true, newTexture);
        std::cout << "[placeBlock] SUCCESS" << std::endl;
        return true;
    }

    std::cout << "[placeBlock] failed: target obstructed or out of bounds" << std::endl;
    return false;
}



namespace fs = std::filesystem;

bool exportWorldToPath(const World& world, const std::string& destinationPath, int boundX, int boundY, int boundZ) {
    fs::path p(destinationPath);
    fs::path dir = p.parent_path();

    if (!dir.empty() && !fs::exists(dir)) {
        try {
            fs::create_directories(dir);
        }
        catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to create directory path: " << e.what() << std::endl;
            return false;
        }
    }

    std::ofstream outFile(destinationPath);
    if (!outFile.is_open()) {
        std::cerr << "Failed to write to file: " << destinationPath << std::endl;
        return false;
    }

    outFile << boundX << " " << boundY << " " << boundZ << "\n";

    for (int x = 0; x < boundX; ++x) {
        for (int y = 0; y < boundY; ++y) {
            for (int z = 0; z < boundZ; ++z) {
                Block b = const_cast<World&>(world).getBlock(x, y, z);
                outFile << b.isSolid << " " << b.texture << "\n";
            }
        }
    }

    outFile.close();
    std::cout << "Successfully exported world to: " << destinationPath << std::endl;
    return true;
}

World importWorldFromPath(const std::string& sourcePath) {
    if (!fs::exists(sourcePath)) {
        std::cerr << "Error: File does not exist at path: " << sourcePath << std::endl;
        return World(0, 0, 0);
    }

    std::ifstream inFile(sourcePath);
    if (!inFile.is_open()) {
        std::cerr << "Failed to open file for importing: " << sourcePath << std::endl;
        return World(0, 0, 0);
    }

    int bx, by, bz;
    inFile >> bx >> by >> bz;

    World loadedWorld(bx, by, bz);

    for (int x = 0; x < bx; ++x) {
        for (int y = 0; y < by; ++y) {
            for (int z = 0; z < bz; ++z) {
                bool solid;
                int tex;
                inFile >> solid >> tex;
                loadedWorld.setBlock(x, y, z, solid, tex);
            }
        }
    }

    inFile.close();
    std::cout << "Successfully imported world from: " << sourcePath << std::endl;
    return loadedWorld;
}


