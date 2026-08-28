#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct EngineSettings 
{
    // Mouse / Cursor state
    float lastX = 400.0f;
    float lastY = 300.0f;
    bool  firstMouse = true;

    // Input toggle flags
    bool menuOpen = false;
    bool ctrlPressedLastFrame = false;

    // Camera settings
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // selection outline color
    glm::vec3 outlineColor = glm::vec3(1.0f, 1.0f, 1.0f);
};

struct GlobalLightSettings {
    float ambientStrength = 0.03f;
    glm::vec3 ambientColor = glm::vec3(1.0f);
    bool dirLightEnabled = true;
    bool pointLightsEnabled = true;
};


inline EngineSettings settings;