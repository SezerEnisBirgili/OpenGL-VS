#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.05f;
const float ZOOM = 45.0f;

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    float TargetYaw;
    glm::vec3 TargetPosition;
    bool IsMoving = false;
    bool IsRotating = false;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() const
    {
        return customLookAt();
    }

    void ProcessKeyboard(const Camera_Movement direction, const float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)  Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT)     Position -= Right * velocity;
        if (direction == RIGHT)    Position += Right * velocity;
        if (direction == DOWN)     Position -= Up    * velocity;
        if (direction == UP)       Position += Up    * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw = glm::mod(Yaw + xoffset, 360.0f);
        Pitch += yoffset;

        if (constrainPitch)
        {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }

        updateCameraVectors();
    }

    void ProcessMouseMovementXZ(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        Yaw += xoffset;
        updateCameraVectors();
    }

    void ProcessMouseScroll(const float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 45.0f) Zoom = 45.0f;
    }

    void SetRotationTarget(const float degrees)
    {
        if (IsRotating || IsMoving) return;
        TargetYaw = Yaw + degrees;
        IsRotating = true;
    }

    void UpdateRotation(const float deltaTime)
    {
        if (!IsRotating) return;

        float speed = 90.0f * deltaTime;
        float diff = TargetYaw - Yaw;

        if (abs(diff) <= speed)
        {
            Yaw = TargetYaw;
            IsRotating = false;
        }
        else
        {
            Yaw += glm::sign(diff) * speed;
        }

        updateCameraVectors();
    }

    void SetPositionTarget(const Camera_Movement direction, const float distance)
    {
        if (IsMoving || IsRotating) return;

        if (direction == FORWARD)
            TargetPosition = Position + Front * distance;
        if (direction == BACKWARD)
            TargetPosition = Position - Front * distance;

        IsMoving = true;
    }

    void UpdatePosition(const float deltaTime, const float speed = SPEED)
    {
        if (!IsMoving) return;

        glm::vec3 diff = TargetPosition - Position;
        float distancePerTime = speed * deltaTime;

        if (glm::length(diff) <= distancePerTime)
        {
            Position = TargetPosition;
            IsMoving = false;
        }
        else
        {
            Position += glm::normalize(diff) * distancePerTime;
        }

        updateCameraVectors();
    }

private:
    void updateCameraVectors()
    {
        glm::vec3 front = glm::vec3(0.0f);
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }

    glm::mat4 customLookAt() const
    {
        glm::mat4 rotationMatrix = glm::transpose(
            glm::mat4(glm::vec4(Right, 0),
                glm::vec4(Up, 0),
                glm::vec4(-Front, 0),
                glm::vec4(0, 0, 0, 1)));

        glm::mat4 translationMatrix =
            glm::mat4(glm::vec4(1, 0, 0, 0),
                glm::vec4(0, 1, 0, 0),
                glm::vec4(0, 0, 1, 0),
                glm::vec4(-Position, 1));

        return rotationMatrix * translationMatrix;
    }
};
#endif