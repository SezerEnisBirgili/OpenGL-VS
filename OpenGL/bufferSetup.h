#pragma once
#include <glad/glad.h>
#include "vertexData.h"

struct VAOs
{
    unsigned int cube;
    unsigned int light;
};

// Upload vertex data and configure attribute pointers.
// Returns handles for the cube VAO and light VAO (shared VBO).
inline VAOs setupBuffers(unsigned int& VBO)
{
    VAOs vaos{};

    glGenVertexArrays(1, &vaos.cube);
    glGenBuffers(1, &VBO);

    glBindVertexArray(vaos.cube);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Light VAO reuses the same VBO – only needs position
    glGenVertexArrays(1, &vaos.light);
    glBindVertexArray(vaos.light);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);
    return vaos;
}
