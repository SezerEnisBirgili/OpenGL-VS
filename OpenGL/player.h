
#pragma once

#include <iostream>
#include <glm/glm.hpp>

#include "utils.h"
#include "camera.h"
#include "world.h"

class Player 
{
public:
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    static Player& getInstance() 
    {
        static Player instance;
        return instance;
    }

    void setWorld(World* w) { world = w; }
    void setCamera(const Camera& cam) { camera = cam; }
    Camera& getCamera() { return camera; }

    bool lookingAtBlock(glm::vec3& hit) 
    {
        if (!world) return false;

        return traverseDDA(camera.Position, camera.Front, 
            [this](int x, int y, int z) { return world->isBlockSolid(glm::vec3(x, y, z)); }, hit);
    }

    bool placeBlock()
    {
        if (!world) {
            std::cout << "[placeBlock] failed: World is null" << std::endl;
            return false;
        }

        glm::vec3 hitBlock(0.0f);

        bool hitSomething = lookingAtBlock(hitBlock);

        std::cout << "[placeBlock] hitSomething=" << hitSomething 
                  << " hitBlock=(" << hitBlock.x << "," << hitBlock.y << "," << hitBlock.z << ")" << std::endl;

        if (!hitSomething) return false;

        RaycastHit3D rayHit = intersectRayAABB3D(camera.Position, camera.Front, hitBlock);

        std::cout << "[placeBlock] rayHit.collided=" << rayHit.collided 
                  << " face=" << static_cast<int>(rayHit.face) << std::endl;

        if (!rayHit.collided) {
            std::cout << "[placeBlock] failed: AABB raycast missed" << std::endl; 
            return false; 
        }

        glm::vec3 faceOffset = getFaceOffset(rayHit.face);

        if (faceOffset == glm::vec3(0.0f)) { 
            std::cout << "[placeBlock] failed: face == None" << std::endl; 
            return false; 
        }

        glm::vec3 targetPos = hitBlock + faceOffset;

        bool isWithinBounds = world->isWithinBounds(targetPos);
        bool isBlockSolid   = world->isBlockSolid(targetPos);

        std::cout << "[placeBlock] target placement=(" << targetPos.x << "," << targetPos.y << "," << targetPos.z << ")" 
                  << " inBounds=" << isWithinBounds 
                  << " alreadySolid=" << isBlockSolid << std::endl;

        if (isWithinBounds && !isBlockSolid) {
            world->setBlock(targetPos);
            world->updateInstanceBuffer(); // Updates GPU instance buffer so new block renders immediately
            std::cout << "[placeBlock] SUCCESS" << std::endl;
            return true;
        }

        std::cout << "[placeBlock] failed: target obstructed or out of bounds" << std::endl;

        return false;
    }

private:
    Player() : world(nullptr) {}

    World* world;
    Camera camera;
};
