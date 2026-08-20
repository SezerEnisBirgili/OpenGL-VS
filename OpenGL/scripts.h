#pragma once

#include <GameObject.h>


void spinBehavior(Registry& registry, int self, float time, float dt)
{
    TransformComponent& t = registry.transforms[self];
    t.rotationEuler.y = time;
}