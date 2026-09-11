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
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct WorldMatrixComponent {
    glm::mat4 value = glm::mat4(1.0f);
};

struct ShaderComponent {
    int ShaderId;
};

struct MeshComponent {
    int MeshId;
};

struct ScriptComponent {
    std::function<void(Registry&, int, float, float)> updateFn;
};

struct TextureComponent {
    int textureId;
    std::string path;
};

struct WorldComponent {
    int worldId;
    int shaderId;
    int outlineShaderId;
};

// Math Helpers
glm::vec3 getEulerAngles(const TransformComponent& t);
void setEulerAngles(TransformComponent& t, const glm::vec3& degrees);
void rotateAroundAxis(TransformComponent& t, const glm::vec3& axis, float radians);
glm::mat4 composeMatrix(const TransformComponent& t);

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

    int registerMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    {
        Mesh mesh;
        mesh.indexCount = (int)indices.size();

        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

        mesh.setupInstanceBuffer();

        int id = nextMesh++;
        meshStorage[id] = mesh;
        return id;
    }

    int registerShader(const char* vertexPath, const char* fragmentPath)
    {
        Shader shader = Shader(vertexPath, fragmentPath);
        int id = nextShader++;
        shaderStorage[id] = shader;
        return id;
    }

    int registerOutlineShader(const char* vertexPath, const char* fragmentPath)
    {
        Shader shader = Shader(vertexPath, fragmentPath);
        int id = nextOutlineShader++;
        outlineShaderStorage[id] = shader;
        return id;
    }

    int registerWorld(int x, int y, int z)
    {
        World world(x,y,z);
        int id = nextWorld++;
        worldStorage[id] = world;
        return id;
    }

    std::unordered_map<int, World>                worldStorage;
    std::unordered_map<int, Mesh>                 meshStorage;
    std::unordered_map<int, Shader>               shaderStorage;
    std::unordered_map<int, Shader>               outlineShaderStorage;

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
    EntityBuilder& mesh(int meshId, Shader* s, MaterialComponent mat = {});
    EntityBuilder& script(std::function<void(Registry&, int, float, float)> func);
    EntityBuilder& pointLight(PointLightComponent light);
    EntityBuilder& dirLight(DirLightComponent light);

    operator int() const { return id; }
};

unsigned int loadTexture(Registry& reg, const std::string& path, unsigned int filter = 0, bool genMipMaps = true);

// Function Declarations
int spawnEntity(Registry& reg, const std::string& name, const TransformComponent& transform = TransformComponent{}, int parent = NULL_ENTITY);
void setParent(Registry& reg, int child, int parent);
int getParent(const Registry& reg, int entity);

void addMesh(Registry& reg, int e, int meshId, Shader* shader, const MaterialComponent& material);
void addWorld(Registry& reg, int e, int world, int shader, int outlineShader);
void addTexture(Registry& reg, int e, const std::string& path);
void addPointLight(Registry& reg, int e, const PointLightComponent& light = PointLightComponent{});
void addDirLight(Registry& reg, int e, const DirLightComponent& light = DirLightComponent{});
void addScript(Registry& reg, int e, std::function<void(Registry&, int, float, float)> fn);


void transformSystem(Registry& reg);
void transformEntity(Registry& reg, int e, const glm::mat4& parentWorld);
void scriptSystem(Registry& reg, float time, float dt);