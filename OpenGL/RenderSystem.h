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

    std::vector<InstancedRenderItem> opaque;
    std::vector<RenderItem> transparent;

    void uploadInstancePositions(Mesh* mesh, const std::vector<glm::vec3>& positions);
    void drawMesh(Mesh* mesh);
    void drawMeshInstanced(Mesh* mesh, int count);
    void render(int e, Shader* shader = nullptr);
    void renderInstanced(const std::vector<InstancedRenderItem>& items, Shader* shader = nullptr);
    void uploadLights(const Registry& reg, Shader& shader);
    void uploadDirLights(const Registry& reg, Shader& shader, bool enabled);
    void collectRenderItems();
    void drawOutline();

public:
    RenderSystem(Registry& r, Player& p, EngineSettings es) : reg(r), player(p), engineSettings(es) {}
    void renderFrame();
};