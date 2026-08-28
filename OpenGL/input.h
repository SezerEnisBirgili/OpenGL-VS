#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include "settings.h"
#include "player.h"

enum class MouseState { FREE, TANK };

class InputManager
{
public:
    // -------------------------------------------------------------------------
    // GLFW Callbacks
    // -------------------------------------------------------------------------

    static void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        if (settings.menuOpen) return;

        if (settings.firstMouse)
        {
            settings.lastX = static_cast<float>(xpos);
            settings.lastY = static_cast<float>(ypos);
            settings.firstMouse = false;
            return;
        }

        float xoffset = static_cast<float>(xpos) - settings.lastX;
        float yoffset = settings.lastY - static_cast<float>(ypos);
        
        settings.lastX = static_cast<float>(xpos);
        settings.lastY = static_cast<float>(ypos);

        Player::getInstance()->getCamera().ProcessMouseMovement(xoffset, yoffset);
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return;

        Player::getInstance()->getCamera().ProcessMouseScroll(static_cast<float>(yoffset));
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        if (ImGui::GetIO().WantCaptureMouse) return;

        if (settings.menuOpen) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            Player::getInstance()->placeBlock();
        }
    }

    // -------------------------------------------------------------------------
    // Per-Frame Polling
    // -------------------------------------------------------------------------

    static void processInput(GLFWwindow* window, float deltaTime)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // Toggle UI Menu with Left Control key
        bool ctrlPressed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);

        if (ctrlPressed && !settings.ctrlPressedLastFrame)
        {
            settings.menuOpen = !settings.menuOpen;
            settings.firstMouse = true; // Prevents camera snap upon closing menu

            glfwSetInputMode(window, GLFW_CURSOR, 
                settings.menuOpen ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        settings.ctrlPressedLastFrame = ctrlPressed;

        // Freeze movement when menu is open
        if (settings.menuOpen) return;

        // Process WASD movement directly on Player's Camera
        Camera& camera = Player::getInstance()->getCamera();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)          camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)          camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)          camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)          camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)      camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);
    }

    static void setupCallbacks(GLFWwindow* window)
    {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }
};