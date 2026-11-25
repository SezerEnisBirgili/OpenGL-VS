#include "cameraSystem.h"
#include "core/settings.h"
#include <glm/gtc/matrix_transform.hpp>

CameraComponent* CameraSystem::getActive(Scene& scene)
{
    for (auto& [entity, cam] : scene.cameras)
        if (cam.active) return &cam;
    return nullptr;
}

glm::mat4 CameraSystem::view(Scene& scene)
{
    const CameraComponent* cam = getActive(scene);
    return cam ? cam->view : glm::mat4(1.0f);
}

glm::mat4 CameraSystem::projection(const CameraComponent& c, float aspect)
{
    return glm::perspective(glm::radians(c.fov), aspect, c.nearPlane, c.farPlane);
}

void CameraSystem::update(Scene& scene)
{
    const float aspect = engineSettings.aspect();

    for (auto& [entity, cam] : scene.cameras)
    {
        if (!cam.active) continue;

        glm::mat4 world = scene.transforms.get(entity)->toMat4();

        cam.view           = glm::inverse(world);
        cam.projection     = glm::perspective(glm::radians(cam.fov), aspect, cam.nearPlane, cam.farPlane);
        cam.viewProjection = cam.projection * cam.view;
    }
}