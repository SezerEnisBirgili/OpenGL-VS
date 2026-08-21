#pragma once


#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include "stb_image.h"

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
    glm::vec3 scale = glm::vec3(1.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
};

inline glm::vec3 getEulerAngles(const TransformComponent& t) 
{
    return glm::degrees(glm::eulerAngles(t.rotation));
}

inline void setEulerAngles(TransformComponent& t, const glm::vec3& degrees) 
{
    t.rotation = glm::quat(glm::radians(degrees));
}

inline void rotateAroundAxis(TransformComponent& t, const glm::vec3& axis, float radians) 
{
    t.rotation = glm::angleAxis(radians, glm::normalize(axis)) * t.rotation;
}

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

struct TextureComponent 
{
    unsigned int textureId = 0;
    unsigned int textureFilter = 0;
    bool generateMipMaps = true;
    std::string name;
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

    int createTexture(const std::string& path, unsigned int filter, bool genMipMaps, const std::string& textureName)
    {
        int width, height, nrChannels;
        stbi_set_flip_vertically_on_load(true);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

        if (!data)
        {
            std::cout << "Failed to load texture '" << path << "': "
                << stbi_failure_reason() << std::endl;
            return -1; // fail code nothing inserted into textures map
        }

        GLenum format = GL_RGB;
        if (nrChannels == 1) { format = GL_RED; }
        else if (nrChannels == 3) { format = GL_RGB; }
        else if (nrChannels == 4) { format = GL_RGBA; }

        int t = nextTexture++;
        TextureComponent& comp = textures[t];
        comp.textureFilter = filter;
        comp.generateMipMaps = genMipMaps;
        comp.name = textureName;

        glGenTextures(1, &comp.textureId);
        glBindTexture(GL_TEXTURE_2D, comp.textureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        if (filter == 0)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMipMaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMipMaps ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        if (genMipMaps) glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        return t;
    }


    unsigned int getTextureId(int handle) const
    {
        auto it = textures.find(handle);
        return it != textures.end() ? it->second.textureId : 0;
    }

    std::unordered_map<int, TransformComponent>    transforms;
    std::unordered_map<int, WorldMatrixComponent>  worldMatrices;
    std::unordered_map<int, HierarchyComponent>    hierarchy;
    std::unordered_map<int, MaterialComponent>     materials;
    std::unordered_map<int, RenderableComponent>   renderables;
    std::unordered_map<int, ScriptComponent>       scripts;
    std::unordered_map<int, PointLightComponent>   pointLights;
    std::unordered_map<int, DirLightComponent>     dirLights;
    std::unordered_map<int, TextureComponent>      textures;
    std::unordered_map<int, std::string>           names;

private:
    int nextEntity = 1; // 0 reserved as NULL_ENTITY
    int nextTexture = 1; // 0 for missing texture
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

inline int getParent(const Registry& reg, int entity)
{
    auto it = reg.hierarchy.find(entity);
    if (it != reg.hierarchy.end()) {
        return it->second.parent;
    }
    return NULL_ENTITY;
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
    m = m * glm::mat4_cast(t.rotation);
    m = glm::scale(m, t.scale);
    return m;
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

inline void renderSystem(Registry& reg, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& viewPos, unsigned int fallbackDiffuse, unsigned int fallbackSpecular, const GlobalLightSettings& globalLights = {}) {

    std::vector<GpuPointLight> lights = lightingSystem(reg);

    for (auto& [entity, renderable] : reg.renderables) 
    {
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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture ? mat.diffuseTexture : fallbackDiffuse);
        renderable.shader->setInt("material.diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat.specularTexture ? mat.specularTexture : fallbackSpecular);
        renderable.shader->setInt("material.specular", 1);

        renderable.mesh->draw();
    }
}