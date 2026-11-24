#pragma once

#include "componentStore.h"
#include "entities/hierarchy.h"
#include "renderer/transform.h"
#include "renderer/components.h"

class Scene {
public:
    int create()
    {
        int e = next++;
        hierarchy.add(e);
        return e;
    }

    void destroy(int e)
    {
        hierarchy.remove(e);
        transforms.remove(e);
        renderables.remove(e);
        cameras.remove(e);
        lights.remove(e);
        names.remove(e);
    }

    Hierarchy hierarchy;
    ComponentStore<Transform>  transforms;
    ComponentStore<Renderable> renderables;
    ComponentStore<Camera>     cameras;
    ComponentStore<Light>      lights;
    ComponentStore<Name>       names;

private:
    int next = 0;
};
