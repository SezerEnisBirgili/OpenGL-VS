#pragma once

#include <vector>
#include <glm/glm.hpp>

struct RenderItem {
    int       e;
    glm::mat4 model;
};


struct InstancedRenderItem {
    int                     e;
    std::vector<glm::vec3>  positions;
};

class IRenderable {
public:
    virtual ~IRenderable() = default;

    virtual void collectRenderItems(std::vector<RenderItem>& out, const glm::mat4& parentTransform) const = 0;

    virtual void collectInstancedRenderItems(std::vector<InstancedRenderItem>& out, const glm::mat4& parentTransform) const {}
};