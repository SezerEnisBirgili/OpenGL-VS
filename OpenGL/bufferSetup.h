#pragma once
#include <glad/glad.h>
#include "vertexData.h"

class Mesh {
private:
    unsigned int VAO = 0, VBO = 0;
    int vertexCount = 0;

    int strideInFloats(const std::vector<int>& layout)
    {
        int total = 0;
        for (const int& componentSize : layout) total += componentSize;
        return total;
    }

    void applyLayout(const std::vector<int>& layout)
    {
        int stride = strideInFloats(layout) * sizeof(float);
        int offset = 0;

        for (size_t i = 0; i < layout.size(); ++i)
        {
            if (layout[i] == 0) { break; } // acoounting for {3, 0, 0} for only position
            glVertexAttribPointer((GLuint)i, layout[i], GL_FLOAT, false, stride, (void*)offset);
            glEnableVertexAttribArray((GLuint)i);
            offset += layout[i] * sizeof(float);
        }
    }

    void release()
    {
        if (VBO) glDeleteBuffers(1, &VBO);
        if (VAO) glDeleteVertexArrays(1, &VAO);
        VAO = 0, VBO = 0;
    }

public:
    Mesh(const std::vector<float>& vertexData, const std::vector<int>& layout)
    {
        vertexCount = (int)vertexData.size() / strideInFloats(layout);

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

        applyLayout(layout);
        glBindVertexArray(0);
    }

    void draw() const
    {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    unsigned int getVAO() const { return VAO; }

    ~Mesh() { release(); }
};