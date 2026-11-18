#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <utility>
#include <cstddef>

#include "vertex.h"

class Mesh {
public:
    unsigned int VAO = 0, VBO = 0, EBO = 0;
    unsigned int instanceVBO = 0;
    int indexCount = 0;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    MaterialComponent material;

    Mesh() = default;

    Mesh(std::string path) 
    {
        
    }

    Mesh(const std::vector<Vertex>& verts, const std::vector<unsigned int>& inds, MaterialComponent mat = MaterialComponent{}) 
        : vertices(verts), indices(inds), material(mat) {
        indexCount = indices.size();
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
            cleanup();

            VAO         = std::exchange(other.VAO, 0);
            VBO         = std::exchange(other.VBO, 0);
            EBO         = std::exchange(other.EBO, 0);
            instanceVBO = std::exchange(other.instanceVBO, 0);
            indexCount  = std::exchange(other.indexCount, 0);

            vertices = std::move(other.vertices);
            indices  = std::move(other.indices);
        }

        return *this;
    }

    void sendToGPU() 
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