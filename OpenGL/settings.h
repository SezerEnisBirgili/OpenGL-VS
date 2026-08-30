#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct EngineSettings
{
    // Screen dimensions
    unsigned int screenWidth = 1024;
    unsigned int screenHeight = 1024;

    // Projection settings
    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    // Mouse state
    float lastX = 400.0f;
    float lastY = 300.0f;
    bool  firstMouse = true;

    // Toggle Flags
    bool menuOpen = false;
    bool ctrlPressedLastFrame = false;

    // outline 
    glm::vec3 outlineColor = glm::vec3(1.0f);

    glm::mat4 getProjectionMatrix() const { return glm::perspective(glm::radians(fov), (float)screenWidth / screenHeight, nearPlane, farPlane); }
};

struct GlobalLightSettings {
    float ambientStrength = 0.03f;
    glm::vec3 ambientColor = glm::vec3(1.0f);
    bool dirLightEnabled = true;
    bool pointLightsEnabled = true;
};

inline EngineSettings engineSettings;
inline GlobalLightSettings lightSettings;