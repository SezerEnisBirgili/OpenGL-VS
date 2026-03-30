#pragma once
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.h"
#include "camera.h"
#include "enum.h"

class AppState 
{
    bool lShiftPressedLastFrame = false;
    bool wPressedLastFrame = false;
    bool sPressedLastFrame = false;
    bool aPressedLastFrame = false;
    bool dPressedLastFrame = false;
    bool TOGGLE_MENU = false;

    // Mouse state
    MouseState& a_mouseState;
    Camera& a_camera;
    glm::mat4& a_projection;

    float lastX = 400.0f;
    float lastY = 300.0f;
    bool  firstMouse = true;

public:

    AppState(MouseState& mouseState, Camera& camera, glm::mat4& projection) : 
        a_mouseState(mouseState), a_camera(camera), a_projection(projection) {}

    // -------------------------------------------------------------------------
    // Callbacks
    // -------------------------------------------------------------------------
    void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    inline void mouse_callback(GLFWwindow* /*window*/, double xpos, double ypos)
    {
        if (TOGGLE_MENU) return;

        if (a_mouseState == FREE)
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

            a_camera.ProcessMouseMovement(xoffset, yoffset);
        }
    }

    void scroll_callback(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset, float SCR_WIDTH = 800.0f, float SCR_HEIGHT = 600.0f)
    {
        a_camera.ProcessMouseScroll((float)yoffset);
        a_projection = glm::perspective(
            glm::radians(a_camera.Zoom),
            (float)SCR_WIDTH / SCR_HEIGHT,
            0.1f, 100.0f);
    }

    // -------------------------------------------------------------------------
    // Per-frame input
    // -------------------------------------------------------------------------
    void processInput(GLFWwindow* window, Shader& /*ourShader*/, float deltaTime)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Left-shift toggles the ImGui menu
        bool lShiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        bool aPressed = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        bool dPressed = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        bool wPressed = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        bool sPressed = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;

        if (lShiftPressed && !lShiftPressedLastFrame)
        {
            TOGGLE_MENU = !TOGGLE_MENU;
            firstMouse = true;
            glfwSetInputMode(window, GLFW_CURSOR,
                TOGGLE_MENU ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        lShiftPressedLastFrame = lShiftPressed;

        // Camera movement (only when menu is hidden)
        if (!TOGGLE_MENU && a_mouseState != TANK)
        {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) a_camera.ProcessKeyboard(FORWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) a_camera.ProcessKeyboard(BACKWARD, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) a_camera.ProcessKeyboard(LEFT, deltaTime);
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) a_camera.ProcessKeyboard(RIGHT, deltaTime);
        }
        else if (a_mouseState == TANK)
        {
            if (wPressed && !wPressedLastFrame) a_camera.SetPositionTarget(FORWARD, 1.0f);
            if (sPressed && !sPressedLastFrame) a_camera.SetPositionTarget(BACKWARD, 1.0f);
            if (aPressed && !aPressedLastFrame) a_camera.SetRotationTarget(-90.0f);
            if (dPressed && !dPressedLastFrame) a_camera.SetRotationTarget(90.0f);
        }
        wPressedLastFrame = wPressed;
        sPressedLastFrame = sPressed;
        aPressedLastFrame = aPressed;
        dPressedLastFrame = dPressed;
    }

    static void framebuffer_size_callback_static(GLFWwindow* window, int width, int height)
    {
        auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        app->framebuffer_size_callback(window, width, height);
    }
    static void mouse_callback_static(GLFWwindow* window, double xpos, double ypos)
    {
        auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        app->mouse_callback(window, xpos, ypos);
    }
    static void scroll_callback_static(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* app = static_cast<AppState*>(glfwGetWindowUserPointer(window));
        app->scroll_callback(window, xoffset, yoffset);
    }

    void setCallbacks(GLFWwindow* window)
    {
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback_static);
        glfwSetCursorPosCallback(window, mouse_callback_static);
        glfwSetScrollCallback(window, scroll_callback_static);
    }
};