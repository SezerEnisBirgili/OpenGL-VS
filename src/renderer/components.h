// components.h
#pragma once

#include <string>
#include <glm/glm.hpp>

#include "renderer/material.h"

struct Renderable
{
    Material material;
    int modelId = -1;
    bool isVisible = true;
};

struct CameraComponent
{
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    bool  active = true;

    glm::mat4 view{ 1.0f };
    glm::mat4 projection{ 1.0f };
    glm::mat4 viewProjection{ 1.0f };
};

enum class LightType { Directional, Point, Spot };

struct Light
{
    LightType type = LightType::Point;
    glm::vec3 color{ 1.0f };
    float     intensity = 1.0f;

    // point and spot only
    float constant  = 1.0f;
    float linear    = 0.09f;
    float quadratic = 0.032f;

    // spot only (degrees)
    float innerAngle = 20.0f;
    float outerAngle = 30.0f;
};

struct Name
{
    std::string value;
};