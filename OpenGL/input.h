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
    InputManager(Registry& reg, Player& p) : player(p), registry(reg) {}

    // -------------------------------------------------------------------------
    // GLFW Callbacks (Must be static to match C-style function pointers)
    // -------------------------------------------------------------------------

    static void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        if (engineSettings.menuOpen) return;

        if (engineSettings.firstMouse)
        {
            engineSettings.lastX = static_cast<float>(xpos);
            engineSettings.lastY = static_cast<float>(ypos);
            engineSettings.firstMouse = false;
            return;
        }

        float xoffset = static_cast<float>(xpos) - engineSettings.lastX;
        float yoffset = engineSettings.lastY - static_cast<float>(ypos);

        engineSettings.lastX = static_cast<float>(xpos);
        engineSettings.lastY = static_cast<float>(ypos);

        self->player.getCamera().ProcessMouseMovement(xoffset, yoffset);
    }

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
        if (ImGui::GetIO().WantCaptureMouse) return;

        self->player.getCamera().ProcessMouseScroll(static_cast<float>(yoffset));
    }

    static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
        if (!self) return;

        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        if (ImGui::GetIO().WantCaptureMouse) return;

        if (engineSettings.menuOpen) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            self->player.placeBlock();
        }
    }

    // -------------------------------------------------------------------------
    // Per-Frame Polling
    // -------------------------------------------------------------------------

    void processInput(GLFWwindow* window, float deltaTime)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        bool ctrlPressed = (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);

        if (ctrlPressed && !engineSettings.ctrlPressedLastFrame)
        {
            engineSettings.menuOpen = !engineSettings.menuOpen;
            engineSettings.firstMouse = true;

            glfwSetInputMode(window, GLFW_CURSOR,
                engineSettings.menuOpen ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        }
        engineSettings.ctrlPressedLastFrame = ctrlPressed;

        if (engineSettings.menuOpen) return;

        Camera& camera = player.getCamera();

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)          camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)          camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)          camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)          camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)      camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime);
    }

    void setupCallbacks(GLFWwindow* window)
    {
        glfwSetWindowUserPointer(window, this);

        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
    }

private:
    Player& player;
    Registry& registry;
};