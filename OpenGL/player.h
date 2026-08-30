#pragma once

#include <glm/glm.hpp>

#include "utils.h"
#include "camera.h"
#include "world.h"

class Player {
public:
    Player() = default;

    void setWorld(World* w) { world = w; }
    void setCamera(const Camera& cam) { camera = cam; }
    Camera& getCamera() { return camera; }
    bool getHasSelectedBlock() const { return hasSelectedBlock; }
    int  getSelectedBlock() const { return selectedBlock; }
    glm::vec3 getSelectedBlockPos() const { return selectedBlockPos; }

    bool lookingAtBlock(glm::vec3& hit);
    bool placeBlock();

private:
    World* world = nullptr;
    Camera camera{};
    bool hasSelectedBlock = false;
    int selectedBlock = 0;
    glm::vec3 selectedBlockPos = glm::vec3(0.0f);
};