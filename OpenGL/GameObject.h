#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <iostream>
#include <map>

#include "stb_image.h"
#include "mesh.h"
#include "shader.h"
#include "settings.h"
#include "world.h"

constexpr int NULL_ENTITY = 0;

// Forward Declarations
class Mesh;
class Shader;
class World;
class Player;
class Camera;
class Registry;

struct TransformComponent {
    glm::vec3 position{ 0.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
    glm::vec3 scale{ 1.0f };
};

struct MaterialComponent {

    bool isAir         = false;
    bool isTransparent = false;

    int shader = 0;
    int diffuseTexture = 0;
    int specularTexture = 0;

    glm::vec3 color        = glm::vec3(1.0f);
    float     shininess    = 32.0f;
    float     emissive     = 0.0f;
    float     alphaCutoff  = 0.5f;
    float     alpha        = 1.0f;
    bool      isMasked     = false;
};

struct PointLightComponent {
    glm::vec3 color{ 1.0f };
    float intensity{ 1.0f };
    float constant{ 1.0f };
    float linear{ 0.09f };
    float quadratic{ 0.032f };
};

struct DirLightComponent {
    glm::vec3 direction{ -0.2f, -1.0f, -0.3f };
    glm::vec3 color{ 1.0f };
    float intensity{ 1.0f };
};

struct HierarchyComponent {
    int parent = NULL_ENTITY;
    std::vector<int> children;
};
struct GpuPointLight {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 color    = glm::vec3(1.0f);
    float intensity    = 1.0f;
    float constant     = 1.0f;
    float linear       = 0.09f;
    float quadratic    = 0.032f;
};

struct WorldMatrixComponent {
    glm::mat4 value = glm::mat4(1.0f);
};

struct ShaderComponent {
    int ShaderId = 0;
};

struct MeshComponent {
    int MeshId = 0;
};

struct ScriptComponent {
    std::function<void(Registry&, int, float, float)> updateFn;
};

struct TextureComponent {
    int textureId = 0;
    std::string path = "";
};

struct WorldComponent {
    int worldId = 0;
    int shaderId = 0;
    int outlineShaderId = 0;
};

// Math Helpers
glm::vec3 getEulerAngles(const TransformComponent& t);
void setEulerAngles(TransformComponent& t, const glm::vec3& degrees);
void rotateAroundAxis(TransformComponent& t, const glm::vec3& axis, float radians);
glm::mat4 composeMatrix(const TransformComponent& t);

unsigned int loadTexture(Registry& reg, const std::string& path, unsigned int filter = 0, bool genMipMaps = true);

// Function Declarations
int spawnEntity(Registry& reg, const std::string& name, const TransformComponent& transform = TransformComponent{}, int parent = NULL_ENTITY);
void setParent(Registry& reg, int child, int parent);
int getParent(const Registry& reg, int entity);

void addMesh(Registry & reg, int e, int meshId, int shader, const MaterialComponent & material);
void addWorld(Registry& reg, int e, int world, int shader, int outlineShader);
void addTexture(Registry& reg, int e, const std::string& path);
void addPointLight(Registry& reg, int e, const PointLightComponent& light = PointLightComponent{});
void addDirLight(Registry& reg, int e, const DirLightComponent& light = DirLightComponent{});
void addScript(Registry& reg, int e, std::function<void(Registry&, int, float, float)> fn);


void transformSystem(Registry& reg);
void transformEntity(Registry& reg, int e, const glm::mat4& parentWorld);
void scriptSystem(Registry& reg, float time, float dt);

class Registry {
public:
    Registry() = default;
    Registry(const Registry&) = delete;
    Registry& operator=(const Registry&) = delete;
    Registry(Registry&&) noexcept = default;
    Registry& operator=(Registry&&) noexcept = default;

    int getFallbackDiffuse() const { return fallbackDiffuse; }
    int getFallbackSpecular() const { return fallbackSpecular; }

    int create(const std::string& name = "") {
        int e = nextEntity++;
        names[e] = name;
        return e;
    }

    int registerMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, std::string meshName)
    {
        // implement either hashing or file based cache
        if (auto it = meshCache.find(meshName); it != meshCache.end()) { return it->second; }

        Mesh mesh(vertices, indices);

        int id = nextMesh++;
        meshStorage[id] = mesh;
        meshCache[meshName] = id;

        return id;
    }

    int registerShader(const char* vertexPath, const char* fragmentPath)
    {
        std::string combined = std::string(vertexPath) + " | " + std::string(fragmentPath);
        if (auto it = shaderCache.find(combined); it != shaderCache.end()) { return it->second; }

        Shader shader = Shader(vertexPath, fragmentPath);

        int id = nextShader++;
        shaderStorage[id] = shader;
        shaderCache[combined] = id;

        return id;
    }

    int registerOutlineShader(const char* vertexPath, const char* fragmentPath)
    {
        std::string combined = std::string(vertexPath) + " | " + std::string(fragmentPath);
        if (auto it = outlineShaderCache.find(combined); it != outlineShaderCache.end()) { return it->second; }

        Shader shader = Shader(vertexPath, fragmentPath);

        int id = nextOutlineShader++;
        outlineShaderStorage[id] = shader;
        outlineShaderCache[combined] = id;

        return id;
    }

    int registerWorld(int x, int y, int z)
    {
        World world = World(this, x,y,z);
        int id = nextWorld++;
        worldStorage[id] = world;
        return id;
    }

    World* getWorld(int e) { return &worldStorage.at(getWorldId(e)); }
    int getWorldId(int e) { return worlds.at(e).worldId; }
    World* getWorldWithId(int w) {return &worldStorage.at(w); }

    Shader* getShader(int e) { return &outlineShaderStorage.at(getShaderId(e)); }
    int getShaderId(int e) { return outlineShaders.at(e).ShaderId; }
    Shader* getShaderWithId(int s) {return &outlineShaderStorage.at(s); }

    Shader* getOutlineShader(int e) { return &shaderStorage.at(getOutlineShaderId(e)); }
    int getOutlineShaderId(int e) { return shaders.at(e).ShaderId; }
    Shader* getOutlineShaderWithId(int s) {return &shaderStorage.at(s); }

    Mesh* getMesh(int e) { return &meshStorage.at(getMeshId(e)); }
    int getMeshId(int e) { return meshes.at(e).MeshId; }
    Mesh* getMeshWithId(int m) {return &meshStorage.at(m); }

    MaterialComponent getMaterial(int e) { return materials.at(e); }

    std::unordered_map<int, World>                worldStorage;
    std::unordered_map<int, Mesh>                 meshStorage;
    std::unordered_map<int, Shader>               shaderStorage;
    std::unordered_map<int, Shader>               outlineShaderStorage;

    std::unordered_map<std::string, int>          meshCache; 
    std::unordered_map<std::string, int>          shaderCache; 
    std::unordered_map<std::string, int>          outlineShaderCache; 

    std::unordered_map<int, TransformComponent>   transforms;
    std::unordered_map<int, WorldMatrixComponent> worldMatrices;
    std::unordered_map<int, WorldComponent>       worlds;
    std::unordered_map<int, HierarchyComponent>   hierarchy;
    std::unordered_map<int, MaterialComponent>    materials;
    std::unordered_map<int, MeshComponent>        meshes;
    std::unordered_map<int, ShaderComponent>      shaders;
    std::unordered_map<int, ShaderComponent>      outlineShaders;
    std::unordered_map<int, ScriptComponent>      scripts;
    std::unordered_map<int, TextureComponent>     textures;
    std::unordered_map<std::string, int>          pathToEntity;
    std::unordered_map<int, PointLightComponent>  pointLights;
    std::unordered_map<int, DirLightComponent>    dirLights;
    std::unordered_map<int, std::string>          names;

    std::vector<int> renderableEntities;
    std::vector<int> renderableWorlds;

private:
    int nextOutlineShader = 1;
    int nextShader = 1;
    int nextWorld = 1;
    int nextMesh = 1;
    int nextEntity = 1;
    int fallbackDiffuse = 1;
    int fallbackSpecular = 2;
};

struct EntityBuilder {
    Registry& reg;
    int id;

    static EntityBuilder create(Registry& reg, const std::string& name, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), int parent = NULL_ENTITY);
    EntityBuilder& mesh(int meshId, int s, MaterialComponent mat = {});
    EntityBuilder& script(std::function<void(Registry&, int, float, float)> func);
    EntityBuilder& pointLight(PointLightComponent light);
    EntityBuilder& dirLight(DirLightComponent light);

    operator int() const { return id; }
};

struct WorldBuilder {
    Registry& reg;
    int e;
    World* worldPtr;

    static WorldBuilder create(Registry& reg, const std::string& name, int chunkX = 16, int chunkY = 16, int chunkZ = 16, int worldShader = 0, int outlineShader = 0) {
        int storedWorldId = reg.registerWorld(chunkX, chunkY, chunkZ);
        int worldEntityId = reg.create(name);

        addWorld(reg, worldEntityId, storedWorldId, worldShader, outlineShader);

        World* ptr = reg.getWorld(worldEntityId);
        return WorldBuilder{ reg, worldEntityId, ptr };
    }

    WorldBuilder& platform(int blockEntityId, int width, int length) {
        if (worldPtr) { worldPtr->createPlatform(blockEntityId, width, length); }
        return *this;
    }

    WorldBuilder& outlineColor(const glm::vec3& color) {
        if (worldPtr) { worldPtr->setOutlineColor(color); }
        return *this;
    }

    operator int() const { return e; }
};