#pragma once

#include <glm/glm.hpp>
#include "scene.h"

class CameraSystem
{ 
public:
    void update(Scene& scene);
    CameraComponent* getActive(Scene& scene);
    Transform* getActiveCameraPos(Scene& scene);

    glm::mat4 view(Scene& scene);
    glm::mat4 projection(const CameraComponent& cam, float aspect);
};