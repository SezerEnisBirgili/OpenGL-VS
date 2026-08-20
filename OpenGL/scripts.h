#pragma once

#include <GameObject.h>


inline auto spinBehavior(float speed = 1.0f)
{
    return [speed](Registry& registry, int self, float time, float dt)
        {
            TransformComponent& t = registry.transforms[self];
            t.rotationEuler.y = time * speed;
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

            t.rotationEuler.z = glm::radians(axialTiltDeg);
            t.rotationEuler.y = time * spinSpeed;
        };
}