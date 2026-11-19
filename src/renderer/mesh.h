
#include <vector>
#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "vertex.h"

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::string diffuseTexturePath;
    std::string specularTexturePath;
};
