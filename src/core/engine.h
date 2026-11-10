#pragma once

#include "entities/GameObject.h"
#include "entities/player.h"
#include "renderer/RenderSystem.h"
#include "core/input.h"

class Engine {
public:
    Engine(EngineSettings settings) : renderSystem(registry, player, settings), inputManager(registry, player) {}

    Registry& getRegistry() { return registry; }
    Player&   getPlayer()   { return player; }
    InputManager& getInputManager() { return inputManager; }

    void update(float time, float dt) {
        transformSystem(registry);
        scriptSystem(registry, time, dt);
    }

    void render() { renderSystem.renderFrame(); }

private:
    ///-REQUIRED-/////////
    Registry     registry;
    Player       player;
    //////////////////////
    RenderSystem renderSystem;
    InputManager inputManager;
};