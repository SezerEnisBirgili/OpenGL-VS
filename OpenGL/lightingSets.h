#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float shininess;
};

struct Light {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    glm::vec3 position;
    glm::vec3 direction;

    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;
};

inline Material material = {
    glm::vec3(0.00f, 0.03f, 0.05f), // ambient
    glm::vec3(0.08f, 0.28f, 0.45f), // diffuse
    glm::vec3(0.60f, 0.70f, 0.80f), // specular
    19.2f                           // shininess
};

inline Light light = {
    glm::vec3(0.05f, 0.05f, 0.04f), // ambient
    glm::vec3(1.00f, 0.98f, 0.92f), // diffuse
    glm::vec3(1.00f, 1.00f, 0.95f), // specular

    glm::vec3(0.0f, 0.0f, 0.0f),    // position
    glm::vec3(-0.2f, -1.0f, -0.3f), // direction

    1.0f,                           // constant
    0.09f,                          // linear
    0.032f,                         // quadratic

    glm::cos(glm::radians(12.5f)),  // cutOff
    glm::cos(glm::radians(17.5f))   // outerCutOff
};
