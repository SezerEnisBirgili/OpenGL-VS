#pragma once

#include "GameObject.h"
#include "imgui_internal.h"
#include "RenderSystem.h"
#include "input.h"

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