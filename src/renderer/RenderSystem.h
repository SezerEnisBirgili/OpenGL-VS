#pragma once

#include "mesh.h"
#include "renderer/scene.h"
#include "shader.h"
#include "renderer/components.h"
#include "renderer/registry.h"

#include <vector>

struct DrawItem {
    const Mesh*     mesh;
    const Material* material;
    glm::mat4       model;
    int             entity;
};

struct FrameData {
    glm::mat4 view, projection;
    glm::vec3 camPos;
};

struct GpuDirLight {
    glm::vec3 direction;
    glm::vec3 color;
    float     intensity;
};

struct GpuPointLight
{
    glm::vec3 position{ 0.0f };
    glm::vec3 color{ 1.0f };
    float     intensity = 1.0f;

    float constant  = 1.0f;
    float linear    = 0.09f;
    float quadratic = 0.032f;
};

struct LightItem
{
    Light light;
    glm::vec3 position{ 0.0f };
    glm::vec3 direction{ 0.0f, 0.0f, -1.0f };
};

class RenderSystem {
private:
    Registry& reg;
    Scene& scene;
    std::unordered_map<int, glm::mat4> worldMatrices;
    std::vector<DrawItem> opaque;
    std::vector<DrawItem> transparent;
    std::vector<LightItem> lights;
    std::vector<LightItem> pointLights;
    std::vector<LightItem> dirLights;
    std::vector<GpuPointLight> pointLightsGPU;
    std::vector<GpuDirLight>   dirLightsGPU;

    void uploadInstancePositions(Mesh* mesh, const std::vector<glm::vec3>& positions);
    void drawMesh(const Mesh* mesh);
    void render(const DrawItem& item, int s = 0);
    //void drawMeshInstanced(Mesh* mesh, int count);
    //void renderInstanced(const std::vector<InstancedRenderItem>& items, int shader = 0);
    void uploadLights(Shader& shader);
    void uploadDirLights(Shader& shader);
    void collectRenderItems();
    void collectPointLights();
    void collectGpuLights();
    void drawOutline();

    void applyDebugState();
    void bindFrameUniforms(Shader& s);

    void drawOpaquePass();
    void drawTransparentPass();
     
    // helpers
    void addLight(int e, const glm::vec3& worldPos);

public:
    RenderSystem(Registry& r, Scene& s) : reg(r), scene(s) {}

    void draw();
};