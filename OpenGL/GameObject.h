#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <unordered_map>
#include <string>

#include "shader.h"
#include "bufferSetup.h"

constexpr int NULL_ENTITY = 0;


class Registry;

struct GlobalLightSettings {
    float ambientStrength = 0.03f;
    glm::vec3 ambientColor = glm::vec3(1.0f);
    bool dirLightEnabled = true;
    bool pointLightsEnabled = true;
};

struct TransformComponent 
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotationEuler = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};

struct WorldMatrixComponent 
{
    glm::mat4 value = glm::mat4(1.0f);
};

struct HierarchyComponent 
{
    int parent = 0;
    std::vector<int> children;
};

struct MaterialComponent 
{
    glm::vec3 color = glm::vec3(1.0f);
    float shininess = 32.0f;
    float emissive = 0.0f;
    unsigned int diffuseTexture = 0;
    unsigned int specularTexture = 0;
};

struct RenderableComponent
{
    const Mesh* mesh = nullptr;
    Shader* shader = nullptr;
};


struct ScriptComponent
{
    std::function<void(Registry&, int, float, float)> updateFn;
};

struct PointLightComponent {
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};

struct DirLightComponent {
    glm::vec3 direction = glm::normalize(glm::vec3(-0.3f, -1.0f, -0.2f));
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
};

struct GpuPointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};


class Registry {
public:
    int create(const std::string& name = "") 
    {
        int e = nextEntity++;
        names[e] = name;
        return e;
    }

    std::unordered_map<int, TransformComponent>    transforms;
    std::unordered_map<int, WorldMatrixComponent>  worldMatrices;
    std::unordered_map<int, HierarchyComponent>    hierarchy;
    std::unordered_map<int, MaterialComponent>     materials;
    std::unordered_map<int, RenderableComponent>   renderables;
    std::unordered_map<int, ScriptComponent>       scripts;
    std::unordered_map<int, PointLightComponent>   pointLights;
    std::unordered_map<int, DirLightComponent>     dirLights;
    std::unordered_map<int, std::string>           names;

private:
    int nextEntity = 1; // 0 reserved as NULL_ENTITY
};

inline int spawnEntity(
    Registry& reg,
    const std::string& name,
    const TransformComponent& transform = TransformComponent{},
    int parent = NULL_ENTITY)
{
    int e = reg.create(name);
    reg.transforms[e] = transform;
    reg.hierarchy[e] = { parent, {} };

    if (parent != NULL_ENTITY) {
        reg.hierarchy[parent].children.push_back(e);
    }
    return e;
}

inline void setParent(Registry& reg, int child, int parent)
{
    HierarchyComponent& childHierarchy = reg.hierarchy[child];

    // detach from old parent's children list, if any
    if (childHierarchy.parent != NULL_ENTITY) {
        auto& oldSiblings = reg.hierarchy[childHierarchy.parent].children;
        oldSiblings.erase(std::remove(oldSiblings.begin(), oldSiblings.end(), child),
            oldSiblings.end());
    }

    childHierarchy.parent = parent;
    if (parent != NULL_ENTITY) {
        reg.hierarchy[parent].children.push_back(child);
    }
}

inline void addMesh(
    Registry& reg,
    int e,
    const Mesh* mesh,
    Shader* shader,
    const MaterialComponent& material = MaterialComponent{})
{
    reg.renderables[e] = { mesh, shader };
    reg.materials[e] = material;
}

inline void addPointLight(
    Registry& reg,
    int e,
    const PointLightComponent& light = PointLightComponent{})
{
    reg.pointLights[e] = light;
}

inline void addDirLight(
    Registry& reg,
    int e,
    DirLightComponent light = DirLightComponent{})
{
    light.direction = glm::normalize(light.direction);
    reg.dirLights[e] = light;
}

inline void addScript(Registry& reg, int e, std::function<void(Registry&, int, float, float)> fn)
{
    reg.scripts[e] = { std::move(fn) };
}

inline glm::mat4 composeMatrix(const TransformComponent& t) 
{
    glm::mat4 m = glm::translate(glm::mat4(1.0f), t.position);
    m = glm::rotate(m, t.rotationEuler.y, glm::vec3(0, 1, 0));
    m = glm::rotate(m, t.rotationEuler.x, glm::vec3(1, 0, 0));
    m = glm::rotate(m, t.rotationEuler.z, glm::vec3(0, 0, 1));
    m = glm::scale(m, t.scale);
    return m;
}

inline void transformSystem(Registry& reg, int e, const glm::mat4& parentWorld) 
{
    glm::mat4 local = composeMatrix(reg.transforms[e]);
    glm::mat4 world = parentWorld * local;
    reg.worldMatrices[e].value = world;

    auto it = reg.hierarchy.find(e);
    if (it != reg.hierarchy.end()) 
    {
        for (int child : it->second.children) 
        {
            transformSystem(reg, child, world);
        }
    }
}

inline void scriptSystem(Registry& reg, float time, float dt) 
{
    for (auto& [entity, script] : reg.scripts) 
    {
        if (script.updateFn) script.updateFn(reg, entity, time, dt);
    }
}

inline std::vector<GpuPointLight> lightingSystem(Registry& reg) 
{
    std::vector<GpuPointLight> lights;

    for (auto& [entity, light] : reg.pointLights) {
        glm::vec3 worldPos = glm::vec3(reg.worldMatrices[entity].value[3]);
        lights.push_back({ worldPos, light.color, light.intensity, light.constant, light.linear, light.quadratic});
    }
    return lights;
}

inline bool dirLightSystem(Registry& reg, glm::vec3& outDirection, glm::vec3& outColor, float& outIntensity) {
    if (reg.dirLights.empty()) return false;
    const auto& [entity, light] = *reg.dirLights.begin();
    outDirection = light.direction;
    outColor = light.color;
    outIntensity = light.intensity;
    return true;
}

inline void uploadLights(Shader& shader, const std::vector<GpuPointLight>& lights) {
    shader.setInt("numPointLights", (int)lights.size());
    for (size_t i = 0; i < lights.size(); ++i) {
        std::string base = "pointLights[" + std::to_string(i) + "].";
        shader.setVec3(base + "position", lights[i].position);
        shader.setVec3(base + "color", lights[i].color);
        shader.setFloat(base + "intensity", lights[i].intensity);
        shader.setFloat(base + "constant", lights[i].constant);
        shader.setFloat(base + "linear", lights[i].linear);
        shader.setFloat(base + "quadratic", lights[i].quadratic);
    }
}

inline void uploadDirLight(Shader& shader, Registry& reg, bool enabled) {
    glm::vec3 direction, color;
    float intensity;
    bool hasSun = dirLightSystem(reg, direction, color, intensity) && enabled;

    shader.setBool("hasDirLight", hasSun);
    if (hasSun) {
        shader.setVec3("dirLight.direction", direction);
        shader.setVec3("dirLight.color", color);
        shader.setFloat("dirLight.intensity", intensity);
    }
}

inline void renderSystem(Registry& reg, const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& viewPos, const GlobalLightSettings& globalLights = {}) {

    std::vector<GpuPointLight> lights = lightingSystem(reg);

    for (auto& [entity, renderable] : reg.renderables) {
        if (!renderable.shader || !renderable.mesh) continue;

        const glm::mat4& world = reg.worldMatrices[entity].value;
        const MaterialComponent& mat = reg.materials[entity];

        renderable.shader->use();

        renderable.shader->setVec3("viewPos", viewPos);
        renderable.shader->setFloat("ambientStrength", globalLights.ambientStrength);
        renderable.shader->setVec3("ambientColor", globalLights.ambientColor);
        renderable.shader->setBool("pointLightsEnabled", globalLights.pointLightsEnabled);

        uploadLights(*renderable.shader, lights);
        uploadDirLight(*renderable.shader, reg, globalLights.dirLightEnabled);

        renderable.shader->setMat4("model", world);
        renderable.shader->setMat4("view", view);
        renderable.shader->setMat4("projection", projection);
        renderable.shader->setVec3("material.color", mat.color);
        renderable.shader->setFloat("material.shininess", mat.shininess);
        renderable.shader->setFloat("material.emissive", mat.emissive);

        if (mat.diffuseTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
            renderable.shader->setInt("material.diffuse", 0);
        }
        if (mat.specularTexture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, mat.specularTexture);
            renderable.shader->setInt("material.specular", 1);
        }

        renderable.mesh->draw();
    }
}