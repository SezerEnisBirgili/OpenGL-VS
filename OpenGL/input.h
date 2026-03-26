#pragma once
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include "globals.h"

// -------------------------------------------------------------------------
// Callbacks
// -------------------------------------------------------------------------
inline void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

inline void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    if (!TOOGLE_MENU)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

inline void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
    projection = glm::perspective(
        glm::radians(camera.Zoom),
        (float)SCR_WIDTH / SCR_HEIGHT,
        0.1f, 100.0f);
}

// -------------------------------------------------------------------------
// Per-frame input
// -------------------------------------------------------------------------
inline void processInput(GLFWwindow* window, Shader& /*ourShader*/)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Left-shift toggles the ImGui menu
    bool lShiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    if (lShiftPressed && !lShiftPressedLastFrame)
    {
        TOOGLE_MENU = !TOOGLE_MENU;
        firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR,
            TOOGLE_MENU ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
    lShiftPressedLastFrame = lShiftPressed;

    // Camera movement (only when menu is hidden)
    if (!TOOGLE_MENU)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}