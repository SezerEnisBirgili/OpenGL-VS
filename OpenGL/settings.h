#pragma once

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
};

inline EngineSettings g_Settings;