#include "player.h"
#include <iostream>

bool Player::lookingAtBlock(glm::vec3& hit)
{
    if (!world) {
        std::cout << "[lookingAtBlock] World pointer is null!" << std::endl;
        return false;
    }

    bool hasHit = traverseDDA(camera.Position, camera.Front,
        [this](int x, int y, int z) {
            return world->isBlockSolid(glm::vec3(x, y, z));
        }, hit);

    if (hasHit) {
        int block = world->getBlock(hit);
        if (block) {
            selectedBlock = block;
            selectedBlockPos = hit;
            hasSelectedBlock = true;
        }
        else {
            std::cout << "[lookingAtBlock] Error: Hit location returned nullptr Block!" << std::endl;
            hasSelectedBlock = false;
        }
    }
    else {
        hasSelectedBlock = false;
    }

    return hasHit;
}

bool Player::placeBlock()
{
    std::cout << "placeBlock: player @ " << this << " world ptr = " << world << std::endl;
    if (!world) {
        std::cout << "[placeBlock] failed: World is null" << std::endl;
        return false;
    }

    if (blockToPlace == 0) {
        std::cout << "[placeBlock] failed: no block selected in palette" << std::endl;
        return false;
    }

    glm::vec3 hitBlock(0.0f);
    bool hitSomething = lookingAtBlock(hitBlock);

    std::cout << "[placeBlock] hitSomething=" << hitSomething
        << " hitBlock=(" << hitBlock.x << "," << hitBlock.y << "," << hitBlock.z << ")" << std::endl;

    if (!hitSomething) return false;

    RaycastHit3D rayHit = intersectRayAABB3D(camera.Position, camera.Front, hitBlock);

    if (!rayHit.collided) {
        std::cout << "[placeBlock] failed: AABB raycast missed" << std::endl;
        return false;
    }
    std::cout << "[placeBlock] rayHit.collided=" << rayHit.collided
        << " face=" << static_cast<int>(rayHit.face) << std::endl;

    glm::vec3 faceOffset = getFaceOffset(rayHit.face);

    if (faceOffset == glm::vec3(0.0f)) {
        std::cout << "[placeBlock] failed: face == None" << std::endl;
        return false;
    }

    glm::vec3 targetPos = hitBlock + faceOffset;

    bool isWithinBounds = world->isWithinBounds(targetPos);
    bool isBlockSolid = world->isBlockSolid(targetPos);

    std::cout << "[placeBlock] target placement=(" << targetPos.x << "," << targetPos.y << "," << targetPos.z << ")"
        << " inBounds=" << isWithinBounds
        << " alreadySolid=" << isBlockSolid << std::endl;

    std::cout << "[placeBlock] block id=" << blockToPlace << std::endl;

    if (isWithinBounds && !isBlockSolid) {
        world->setBlock(blockToPlace, targetPos);
        std::cout << "[placeBlock] SUCCESS" << std::endl;
        return true;
    }

    std::cout << "[placeBlock] failed: target obstructed or out of bounds" << std::endl;
    return false;
}