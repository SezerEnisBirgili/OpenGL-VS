#pragma once

#include "shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Transform {
    glm::vec3 position      = glm::vec3(0.0f);
    glm::vec3 rotationEuler = glm::vec3(0.0f);
    glm::vec3 scale         = glm::vec3(1.0f);

    glm::mat4 toMatrix() const {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), position);

        m = glm::rotate(m, rotationEuler.y, glm::vec3(0, 1, 0));
        m = glm::rotate(m, rotationEuler.x, glm::vec3(1, 0, 0));
        m = glm::rotate(m, rotationEuler.z, glm::vec3(0, 0, 1));
        m = glm::scale(m, scale);

        return m;
    }
};

struct Material {
    glm::vec3 color = glm::vec3(1.0f);
    float shininess = 32.0f;
    unsigned int diffuseTexture = 0;
    unsigned int specularTexture = 0;
};

class IUpdatable {
public:
    virtual ~IUpdatable() = default;

    virtual void update(class GameObject& self, float time, float dt) = 0;
};

class GameObject {
public:
    std::string name;
    Transform transform;
    Material material;

    unsigned int VAO = 0;
    int vertexCount = 0;
    Shader* shader = nullptr;

    GameObject* parent = nullptr;

    std::vector<std::unique_ptr<GameObject>> children;

    std::unique_ptr<IUpdatable> behavior = nullptr;

    GameObject* addChild(std::unique_ptr<GameObject> child) {
        child->parent = this;
        children.push_back(std::move(child));
        return children.back().get();
    }

    void update(float time, float dt) {
        if (behavior) behavior->update(*this, time, dt);
        for (auto& c : children) c->update(time, dt);
    }

    void draw(const glm::mat4& parentWorld, const glm::mat4& view, const glm::mat4& projection) {
        glm::mat4 world = parentWorld * transform.toMatrix();

        if (shader && VAO != 0) {
            shader->use();
            shader->setMat4("model", world);
            shader->setMat4("view", view);
            shader->setMat4("projection", projection);

            shader->setVec3("material.color", material.color);
            shader->setFloat("material.shininess", material.shininess);

            if (material.diffuseTexture) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
                shader->setInt("material.diffuse", 0);
            }
            if (material.specularTexture) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, material.specularTexture);
                shader->setInt("material.specular", 1);
            }

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        }

        for (auto& c : children) c->draw(world, view, projection);
    }
};