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

    void setupInstanceBuffer() {
        glGenBuffers(1, &instanceVBO);
    }
};