#include "transformSystem.h"

static void propagate(Scene& scene, int e, const glm::mat4& parentWorld)
{
    glm::mat4 world = parentWorld;

    if (Transform* t = scene.transforms.get(e))
    {
        world = parentWorld * t->toMat4();
        t->world = world;
    }

    scene.hierarchy.forEachChild(e, [&](int child) { propagate(scene, child, world); });
}

void TransformSystem::update(Scene& scene)
{
    scene.hierarchy.forEachParent([&](int root) { propagate(scene, root, glm::mat4(1.0f)); });
}