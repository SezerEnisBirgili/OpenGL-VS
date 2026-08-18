#pragma once

#include <unordered_map>
#include <string>
#include <shader.h>

struct TextureSlot {
    unsigned int texture;
    std::string uniformName;
};

class Material {
private:
    std::vector<TextureSlot> textures;
    float shininess;

public:
    Material(std::vector<TextureSlot> textures, float shininess = 32.0f) : textures(std::move(textures)), shininess(shininess) {}

    void bind(Shader& shader) const {
        for (size_t i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + (GLenum)i);
            glBindTexture(GL_TEXTURE_2D, textures[i].texture);
            shader.setInt(textures[i].uniformName, (int)i);
        }
        shader.setFloat("material.shininess", shininess);
    }
};

class MaterialRegistry {
private:
    std::unordered_map<int, Material> materials;

public:
    void add(int id, Material material) {
        materials.emplace(id, std::move(material));
    }

    const Material& get(int id) const {
        return materials.at(id);
    }
};