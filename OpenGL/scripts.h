#pragma once

#include <GameObject.h>

inline auto spinBehavior(float speed = 1.0f)
{
    return [speed](Registry& registry, int self, float time, float dt)
        {
            TransformComponent& t = registry.transforms[self];
            t.rotation = glm::angleAxis(time * speed, glm::vec3(0, 1, 0));
        };
}

inline auto earthOrbitBehavior(float orbitSpeed = 1.0f, float spinSpeed = 1.0f, float axialTiltDeg = 0.0f)
{
    return [orbitSpeed, spinSpeed, axialTiltDeg](Registry& registry, int self, float time, float dt)
        {
            TransformComponent& t = registry.transforms[self];

            float radius = glm::length(glm::vec3(t.position.x, 0.0f, t.position.z));
            float orbitAngle = time * orbitSpeed;

            t.position.x = radius * cosf(orbitAngle);
            t.position.z = radius * sinf(orbitAngle);
            
            // locally spin around own axis
            glm::quat tiltQuat = glm::angleAxis(glm::radians(axialTiltDeg), glm::vec3(0, 0, 1));
            glm::vec3 tiltedAxis = tiltQuat * glm::vec3(0, 1, 0);
            glm::quat spinQuat = glm::angleAxis(time * spinSpeed, tiltedAxis);

            t.rotation = spinQuat * tiltQuat;
        };
}