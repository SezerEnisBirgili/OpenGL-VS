#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "camera.h"
#include "lightingSets.h"

// -------------------------------------------------------------------------
// Screen
// -------------------------------------------------------------------------
constexpr unsigned int SCR_WIDTH = 800;
constexpr unsigned int SCR_HEIGHT = 600;

// -------------------------------------------------------------------------
// Shader / asset paths
// -------------------------------------------------------------------------
inline const char* vertexShader[] = { "vPhongShader.vert", "vLightShader.vert" };
inline const char* fragmentShader[] = { "fPhongShader.frag", "fLightShader.frag" };
inline const char* wallFilePath = "platform.txt";

// -------------------------------------------------------------------------
// Timing
// -------------------------------------------------------------------------
inline float deltaTime = 0.0f;
inline float lastFrame = 0.0f;

// -------------------------------------------------------------------------
// ImGui / menu state
// -------------------------------------------------------------------------
bool lShiftPressedLastFrame = false;
bool wPressedLastFrame = false;
bool sPressedLastFrame = false;
bool aPressedLastFrame = false;
bool dPressedLastFrame = false;
inline bool TOGGLE_MENU = false;
inline bool RAINBOW = false;

enum MouseState { FREE, TANK };
const char* const mouseStateArr[] = { "FREE", "TANK"};
MouseState mouseState = MouseState::FREE;

// -------------------------------------------------------------------------
// Camera
// -------------------------------------------------------------------------
inline glm::vec3 cameraPos = glm::vec3(1.0f, 1.0f, -3.0f);
inline glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
inline float     yaw = 90.0f;
inline float     pitch = 0.0f;
inline Camera    camera = Camera(cameraPos, cameraUp, yaw, pitch);

// Mouse state
inline float lastX = 400.0f;
inline float lastY = 300.0f;
inline bool  firstMouse = true;

// -------------------------------------------------------------------------
// Matrices
// -------------------------------------------------------------------------
inline glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 1.0f, 2.0f)), glm::vec3(1.0f));

inline glm::mat4 view = camera.GetViewMatrix();

inline glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
