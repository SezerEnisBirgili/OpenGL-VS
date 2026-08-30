#include "GameObject.h"
#include "world.h"

#include <algorithm>

// ==========================================
// Math Helpers
// ==========================================

glm::vec3 getEulerAngles(const TransformComponent& t) {
    return glm::degrees(glm::eulerAngles(t.rotation));
}

void setEulerAngles(TransformComponent& t, const glm::vec3& degrees) {
    t.rotation = glm::quat(glm::radians(degrees));
}

void rotateAroundAxis(TransformComponent& t, const glm::vec3& axis, float radians) {
    t.rotation = glm::angleAxis(radians, glm::normalize(axis)) * t.rotation;
}

glm::mat4 composeMatrix(const TransformComponent& t) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), t.position);
    m = m * glm::mat4_cast(t.rotation);
    m = glm::scale(m, t.scale);
    return m;
}

// ==========================================
// Entity / Hierarchy
// ==========================================

int spawnEntity(Registry& reg, const std::string& name, const TransformComponent& transform, int parent) {
    int e = reg.create(name);
    reg.transforms[e] = transform;
    reg.hierarchy[e] = { parent, {} };

    if (parent != NULL_ENTITY) {
        reg.hierarchy[parent].children.push_back(e);
    }
    return e;
}

void setParent(Registry& reg, int child, int parent) {
    HierarchyComponent& childHierarchy = reg.hierarchy[child];

    if (childHierarchy.parent != NULL_ENTITY) {
        auto& oldSiblings = reg.hierarchy[childHierarchy.parent].children;
        oldSiblings.erase(std::remove(oldSiblings.begin(), oldSiblings.end(), child), oldSiblings.end());
    }

    childHierarchy.parent = parent;
    if (parent != NULL_ENTITY) {
        reg.hierarchy[parent].children.push_back(child);
    }
}

int getParent(const Registry& reg, int entity) {
    auto it = reg.hierarchy.find(entity);
    if (it != reg.hierarchy.end()) {
        return it->second.parent;
    }
    return NULL_ENTITY;
}

// ==========================================
// Component Attachment
// ==========================================


void addMesh(Registry & reg, int e, int meshId, Shader * shader, const MaterialComponent & material) {
    MaterialComponent mat = material;

    if (mat.diffuseTexture == 0) {
        std::cerr << "[RenderSystem] entity " << e << " (mesh " << meshId << ") has no diffuseTexture set — " << "using fallback diffuse (" << reg.getFallbackDiffuse() << ")\n";
        mat.diffuseTexture = reg.getFallbackDiffuse();
    }

    if (mat.specularTexture == 0) {
        std::cerr << "[RenderSystem] entity " << e << " (mesh " << meshId << ") has no specularTexture set — " << "using fallback specular (" << reg.getFallbackSpecular() << ")\n";
        mat.specularTexture = reg.getFallbackSpecular();
    }

    reg.meshes[e] = { &reg.meshStorage.at(meshId) };
    reg.shaders[e] = { shader };
    reg.materials[e] = mat;
    reg.renderableEntities.push_back(e);
}

void addWorld(Registry& reg, int e, World& world, Shader* shader, Shader* outlineShader) {
    // `world` is expected to already live inside reg.worlds[e] (constructed in place,
    // since World holds a Registry& member and can't be copied/moved in afterward).
    // This just wires up its render-related side tables.
    reg.shaders[e] = { shader };
    reg.outlineShaders[e] = { outlineShader };
    reg.renderableWorlds.push_back(e);
}

void addTexture(Registry& reg, int e, const std::string& path) {
    unsigned int texId = loadTexture(reg, path);
    reg.textures[e] = { (int)texId, path };
    reg.pathToEntity[path] = e;
}

void addPointLight(Registry& reg, int e, const PointLightComponent& light) {
    reg.pointLights[e] = light;
}

void addDirLight(Registry& reg, int e, const DirLightComponent& light) {
    reg.dirLights[e] = light;
}

void addScript(Registry& reg, int e, std::function<void(Registry&, int, float, float)> fn) {
    reg.scripts[e].updateFn = fn;
}

// ==========================================
// EntityBuilder
// ==========================================

EntityBuilder EntityBuilder::create(Registry& reg, const std::string& name, glm::vec3 pos, glm::vec3 scale, int parent) {
    TransformComponent t{ .position = pos, .scale = scale };
    return EntityBuilder{ reg, spawnEntity(reg, name, t, parent) };
}

EntityBuilder& EntityBuilder::mesh(int meshId, Shader* s, MaterialComponent mat) {
    addMesh(reg, id, meshId, s, mat);
    return *this;
}

EntityBuilder& EntityBuilder::script(std::function<void(Registry&, int, float, float)> func) {
    addScript(reg, id, func);
    return *this;
}

EntityBuilder& EntityBuilder::pointLight(PointLightComponent light) {
    addPointLight(reg, id, light);
    return *this;
}

EntityBuilder& EntityBuilder::dirLight(DirLightComponent light) {
    addDirLight(reg, id, light);
    return *this;
}

// ==========================================
// Transform & Script Systems
// ==========================================

void transformEntity(Registry& reg, int e, const glm::mat4& parentWorld) {
    glm::mat4 local = composeMatrix(reg.transforms[e]);
    glm::mat4 world = parentWorld * local;
    reg.worldMatrices[e].value = world;

    auto it = reg.hierarchy.find(e);
    if (it != reg.hierarchy.end()) {
        for (int child : it->second.children) {
            transformEntity(reg, child, world);
        }
    }
}

void transformSystem(Registry& reg) {
    for (const auto& [e, hierarchyComp] : reg.hierarchy) {
        if (hierarchyComp.parent == NULL_ENTITY) {
            transformEntity(reg, e, glm::mat4(1.0f));
        }
    }
}

void scriptSystem(Registry& reg, float time, float dt) {
    for (auto& [entity, script] : reg.scripts) {
        if (script.updateFn) {
            script.updateFn(reg, entity, time, dt);
        }
    }
}

unsigned int loadTexture(Registry& reg, const std::string& path, unsigned int filter, bool genMipMaps) {
    // Reuse an already-loaded texture for the same path instead of re-decoding it.
    auto it = reg.pathToEntity.find(path);
    if (it != reg.pathToEntity.end()) {
        return (unsigned int)reg.textures.at(it->second).textureId;
    }

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Failed to load texture: " << path << " (" << stbi_failure_reason() << ")" << std::endl;
        return 0;
    }

    GLenum format = GL_RGB;
    if (channels == 1)      format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    unsigned int textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   // <-- prevents row-padding corruption on small/odd-width textures

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    if (genMipMaps) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    // filter: 0 = GL_NEAREST (crisp, blocky — good for tiny/flat swatches like fallback textures),
    //         1 = GL_LINEAR  (smooth — good for normal photographic/tiled textures)
    GLint magFilter = (filter == 0) ? GL_NEAREST : GL_LINEAR;
    GLint minFilter;
    if (genMipMaps)
        minFilter = (filter == 0) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
    else
        minFilter = magFilter;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    // Register into the registry so future calls with the same path are deduplicated,
    // and so addTexture()/entities can look it up as a proper TextureComponent later.
    int e = reg.create("texture:" + path);
    reg.textures[e] = { (int)textureId, path };
    reg.pathToEntity[path] = e;

    return textureId;
}