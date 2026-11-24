#pragma once

int spawnModel(Scene& s, Registry& reg, const std::string& path, Transform t)
{
    int modelId = reg.loadModel(path);

    int e = s.create();
    s.transforms.add(e, t);
    s.renderables.add(e, Renderable{ modelId });
    return e;
}

int spawnCamera(Scene& s, Transform t)
{
    int e = s.create();
    s.transforms.add(e, t);
    s.cameras.add(e);
    return e;
}

int spawnLight(Scene& s, Transform t, glm::vec3 color)
{
    int e = s.create();
    s.transforms.add(e, t);
    s.lights.add(e, Light{ color });
    return e;
}