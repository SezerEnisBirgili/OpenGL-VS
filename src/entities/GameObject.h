#pragma once

#include "entities/Hierarchy.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>


#include "stb_image.h"
#include "renderer/transform.h"
#include "renderer/mesh.h"
#include "renderer/model.h"
#include "renderer/texture.h"
#include "renderer/shader.h"
#include "world/world.h"
#include "Entity.h"

// Forward Declarations
class Mesh;
class Shader;
class World;
class Player;
class Camera;
class Registry;

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

struct ScriptComponent {
    std::function<void(Registry&, int, float, float)> updateFn;
};

struct WorldComponent {
    int worldId = 0;
    int shaderId = 0;
    int outlineShaderId = 0;
};

// Math Helpers
glm::vec3 getEulerAngles(const Transform& t);
void setEulerAngles(Transform& t, const glm::vec3& degrees);
void rotateAroundAxis(Transform& t, const glm::vec3& axis, float radians);
glm::mat4 composeMatrix(const Transform& t);

unsigned int loadTexture(Registry& reg, const std::string& path, unsigned int filter = 0, bool genMipMaps = true);

// Function Declarations

void addShader(Registry& reg, int e, int shader);
void addMaterial(Registry& reg, int e, const MaterialComponent& material);
void addBlock(Registry& reg, int e, int meshId, int shader, MaterialComponent& material);
void addWorld(Registry& reg, int e, int world, int shader, int outlineShader);
void addPointLight(Registry& reg, int e, const PointLightComponent& light = PointLightComponent{});
void addDirLight(Registry& reg, int e, const DirLightComponent& light = DirLightComponent{});
void addScript(Registry& reg, int e, std::function<void(Registry&, int, float, float)> fn);


void transformSystem(Registry& reg);
void transformEntity(Registry& reg, int e, const glm::mat4& parentWorld);
void scriptSystem(Registry& reg, float time, float dt);

struct EntityBuilder {
    Registry& reg;
    int id;

    static EntityBuilder create(Registry& reg, const std::string& name, glm::vec3 pos = glm::vec3(0.0f), glm::vec3 scale = glm::vec3(1.0f), int parent = NULL_ENTITY);
    EntityBuilder& renderable(int meshId, int s, MaterialComponent mat = {});
    EntityBuilder& mesh(int meshId, int s, MaterialComponent mat = {});
    EntityBuilder& block(int meshId, int s, MaterialComponent mat = {});
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