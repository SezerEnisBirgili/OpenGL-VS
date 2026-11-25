#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct EngineSettings
{
    // Screen dimensions
    unsigned int screenWidth = 1920;
    unsigned int screenHeight = 1080;

    float aspect() const { return screenHeight == 0 ? 1.0f : (float)screenWidth / screenHeight; }

    // Mouse state
    float lastX = 400.0f;
    float lastY = 300.0f;
    bool  firstMouse = true;

    // Toggle Flags
    bool menuOpen = false;
    bool ctrlPressedLastFrame = false;

    // outline 
    glm::vec3 outlineColor = glm::vec3(1.0f);
};

struct GlobalLightSettings {
    float ambientStrength = 0.1f;
    glm::vec3 ambientColor = glm::vec3(1.0f);
    bool dirLightEnabled = true;
    bool pointLightsEnabled = true;
};

struct DebugSettings {
    bool cullFace = true;
    bool wireframe = false;
    bool showFaceColors = false; // green = front, red = back
};

inline EngineSettings engineSettings;
inline GlobalLightSettings lightSettings;
inline DebugSettings debugSettings;
