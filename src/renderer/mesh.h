
#include <vector>
#include <cstddef>
#include <utility>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "shader.h"
#include "vertex.h"

struct MaterialComponent {
    unsigned int diffuseTexture = 0;
    unsigned int specularTexture = 0;
};

struct Mesh {
    // CPU Data (can be cleared after setupMesh() to save RAM)
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    MaterialComponent material;

    // GPU Handles
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int indexCount = 0;

    Mesh() = default;

    // Upload data to GPU once
    void setupMesh() {
        indexCount = indices.size();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

        // Optional: Free CPU RAM now that data lives on the GPU
        vertices.clear();
        vertices.shrink_to_fit();
        indices.clear();
        indices.shrink_to_fit();
    }

    void Draw(const Shader& shader) const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
        shader.setInt("material.diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, material.specularTexture);
        shader.setInt("material.specular", 1);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept
        : vertices(std::move(other.vertices)),
          indices(std::move(other.indices)),
          material(other.material),
          VAO(other.VAO), VBO(other.VBO), EBO(other.EBO),
          indexCount(other.indexCount) {
        other.VAO = other.VBO = other.EBO = 0;
        other.indexCount = 0;
    }

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            cleanup();
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            material = other.material;
            VAO = other.VAO; VBO = other.VBO; EBO = other.EBO;
            indexCount = other.indexCount;
            other.VAO = other.VBO = other.EBO = 0;
            other.indexCount = 0;
        }
        return *this;
    }

    ~Mesh() { cleanup(); }

private:
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
    }
};
