#pragma once

#include "mesh.h"
#include "shader.h"
#include "IRenderable.h"
#include "entities/GameObject.h"
#include "entities/player.h"
#include "renderer/scene.h"
#include "renderer/registry.h"

#include <vector>

struct RenderItem 
{
    Transform transform;
    Renderable renderable;
};

class RenderSystem {
private:
    Registry& reg;
    Scene& scene;

    std::vector<Renderable> renderables;
    std::vector<Light> lights;

    void uploadInstancePositions(Mesh* mesh, const std::vector<glm::vec3>& positions);
    void drawMesh(int m);
    void drawMeshInstanced(Mesh* mesh, int count);
    void render(Renderable renderable, int s = 0);
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
    RenderSystem(Scene& s, Registry& r, Player& p) : scene(s), reg(r), player(p) {}
    void draw();
};