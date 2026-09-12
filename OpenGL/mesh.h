#pragma once

#include <glad/glad.h>
#include <vector>

#include "vertex.h"
#include "shader.h"

class Mesh {
public:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int instanceVBO = 0;
    int indexCount = 0;

    Mesh() = default;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) 
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

        glGenBuffers(1, &instanceVBO);

        indexCount = static_cast<int>(indices.size());
    }

    ~Mesh() {
        cleanup();
    }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh(Mesh&& other) noexcept {
        *this = std::move(other);
    }

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            cleanup(); // Release existing resources on this object

            VAO = other.VAO;
            VBO = other.VBO;
            EBO = other.EBO;
            instanceVBO = other.instanceVBO;
            indexCount = other.indexCount;

            other.VAO = 0;
            other.VBO = 0;
            other.EBO = 0;
            other.instanceVBO = 0;
            other.indexCount = 0;
        }
        return *this;
    }

private:
    void cleanup() {
        if (VAO) glDeleteVertexArrays(1, &VAO);
        if (VBO) glDeleteBuffers(1, &VBO);
        if (EBO) glDeleteBuffers(1, &EBO);
        if (instanceVBO) glDeleteBuffers(1, &instanceVBO);

        VAO = VBO = EBO = instanceVBO = 0;
        indexCount = 0;
    }
};