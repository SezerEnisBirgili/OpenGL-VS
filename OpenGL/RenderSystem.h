#pragma once

#include "mesh.h"
#include "GameObject.h"
#include "settings.h"
#include "shader.h"
#include "player.h"
#include "IRenderable.h"

#include <vector>

class RenderSystem {
private:
    Player& player;
    Registry& reg;
    EngineSettings engineSettings;

    std::vector<RenderItem> opaque;
    std::vector<RenderItem> transparent;
    std::vector<InstancedRenderItem> instancedOpaque;

    void uploadInstancePositions(Mesh* mesh, const std::vector<glm::vec3>& positions);
    void drawMesh(int m);
    void drawMeshInstanced(Mesh* mesh, int count);
    void render(RenderItem renderable, int s = 0);
    void renderInstanced(const std::vector<InstancedRenderItem>& items, int shader = 0);
    void uploadLights(const Registry& reg, Shader& shader);
    void uploadDirLights(const Registry& reg, Shader& shader, bool enabled);
    void collectRenderItems();
    void drawOutline();

public:
    RenderSystem(Registry& r, Player& p, EngineSettings es) : reg(r), player(p), engineSettings(es) {}
    void renderFrame();
};