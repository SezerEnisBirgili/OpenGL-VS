#pragma once

#include <glm/glm.hpp>
#include <algorithm>

enum class BoxFace3D {
    None,
    Left,   // -X
    Right,  // +X
    Bottom, // -Y
    Top,    // +Y
    Back,   // -Z
    Front   // +Z
};

struct RaycastHit3D {
    bool collided = false;
    float t = -1.0f;
    glm::vec3 point = { 0.0f, 0.0f, 0.0f };
    BoxFace3D face = BoxFace3D::None;
};

template <typename Visitor>
bool traverseDDA(glm::vec3 start, glm::vec3 front, Visitor&& visit, glm::vec3& hit, int LOOP_LIMIT=100)
{
    if (front.x == 0 && front.y == 0 && front.z == 0)
        return false;

    front = glm::normalize(front);

    int x = static_cast<int>(std::floor(start.x));
    int y = static_cast<int>(std::floor(start.y));
    int z = static_cast<int>(std::floor(start.z));

    int dir_x = (front.x > 0) - (front.x < 0);
    int dir_y = (front.y > 0) - (front.y < 0);
    int dir_z = (front.z > 0) - (front.z < 0);

    const float BIG = 1e30f;

    // if front is normalized, scaling factor calculation simplies to this
    float S_x = (front.x != 0) ? std::abs(1.0f / front.x) : INFINITY;
    float S_y = (front.y != 0) ? std::abs(1.0f / front.y) : INFINITY;
    float S_z = (front.z != 0) ? std::abs(1.0f / front.z) : INFINITY;

    float D_x = (dir_x < 0) ? (start.x - x) * S_x : (x + 1 - start.x) * S_x;
    float D_y = (dir_y < 0) ? (start.y - y) * S_y : (y + 1 - start.y) * S_y;
    float D_z = (dir_z < 0) ? (start.z - z) * S_z : (z + 1 - start.z) * S_z;

    for (int looped = 0; looped < LOOP_LIMIT; looped++)
    {
        if (visit(x, y, z))
        {
            hit = glm::vec3(x, y, z);
            return true;
        }

        if (D_x >= D_y) 
        {
            if (D_y >= D_z)  { z += dir_z; D_z += S_z; }
            else             { y += dir_y; D_y += S_y; }
        }

        else if (D_x >= D_z) { z += dir_z; D_z += S_z; }
        else                 { x += dir_x; D_x += S_x; }
    }

    return false;
}

inline RaycastHit3D intersectRayAABB3D(const glm::vec3& start, const glm::vec3& rayDir, const glm::vec3& hitBlock) {

    glm::vec3 min = hitBlock;
    glm::vec3 max = glm::vec3(min.x + 1.0f, min.y + 1.0f, min.z + 1.0f);

    RaycastHit3D hit;

    float tMinX = (min.x - start.x) / rayDir.x;
    float tMaxX = (max.x - start.x) / rayDir.x;
    BoxFace3D nearFaceX = BoxFace3D::Left;
    if (tMinX > tMaxX) {
        std::swap(tMinX, tMaxX);
        nearFaceX = BoxFace3D::Right;
    }

    float tMinY = (min.y - start.y) / rayDir.y;
    float tMaxY = (max.y - start.y) / rayDir.y;
    BoxFace3D nearFaceY = BoxFace3D::Bottom;
    if (tMinY > tMaxY) {
        std::swap(tMinY, tMaxY);
        nearFaceY = BoxFace3D::Top;
    }

    float tMinZ = (min.z - start.z) / rayDir.z;
    float tMaxZ = (max.z - start.z) / rayDir.z;
    BoxFace3D nearFaceZ = BoxFace3D::Back;
    if (tMinZ > tMaxZ) {
        std::swap(tMinZ, tMaxZ);
        nearFaceZ = BoxFace3D::Front;
    }

    float tNear = std::max({ tMinX, tMinY, tMinZ });
    float tFar = std::min({ tMaxX, tMaxY, tMaxZ });

    if (tNear <= tFar && tFar >= 0.0f) {
        hit.collided = true;
        hit.t = (tNear < 0.0f) ? 0.0f : tNear; // If origin is inside the box, t is 0

        hit.point.x = start.x + rayDir.x * hit.t;
        hit.point.y = start.y + rayDir.y * hit.t;
        hit.point.z = start.z + rayDir.z * hit.t;

        if (tNear == tMinX) {
            hit.face = nearFaceX;
        }
        else if (tNear == tMinY) {
            hit.face = nearFaceY;
        }
        else {
            hit.face = nearFaceZ;
        }
        return hit;
    }

    return hit; // Missed
}

inline glm::vec3 getFaceOffset(BoxFace3D face) 
{
    switch (face) 
    {
        case BoxFace3D::Left:   return glm::vec3(-1,  0,  0);
        case BoxFace3D::Right:  return glm::vec3( 1,  0,  0);
        case BoxFace3D::Bottom: return glm::vec3( 0, -1,  0);
        case BoxFace3D::Top:    return glm::vec3( 0,  1,  0);
        case BoxFace3D::Back:   return glm::vec3( 0,  0, -1);
        case BoxFace3D::Front:  return glm::vec3( 0,  0,  1);
        default:                return glm::vec3( 0,  0,  0);
    }
}