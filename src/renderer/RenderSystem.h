#pragma once

#include "mesh.h"
#include "shader.h"
#include "IRenderable.h"
#include "entities/GameObject.h"
#include "entities/player.h"

#include <vector>

struct Renderable {
    int modelID;
    int textureID;
    Transform transform;
};

class RenderSystem {
private:
    Registry& reg;

    std::vector<Renderable> renderables;

    void uploadInstancePositions(Mesh* mesh, const std::vector<glm::vec3>& positions);
    void drawMesh(int m);
    void drawMeshInstanced(Mesh* mesh, int count);
    void render(RenderItem renderable, int s = 0);
    void renderInstanced(const std::vector<InstancedRenderItem>& items, int shader = 0);
    void uploadLights(Shader& shader);
    void uploadDirLights(const Registry& reg, Shader& shader, bool enabled);
    void collectRenderItems();
    void collectPointLights();
    void drawOutline();

    void applyDebugState();
     
    // helpers
    void addLight(int e, const glm::vec3& worldPos);

public:
    RenderSystem(Registry& r, Player& p) : reg(r), player(p) {}
    void renderFrame();
};