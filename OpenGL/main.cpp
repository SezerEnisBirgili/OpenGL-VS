#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "horrorWorld.h"
#include "vertexData.h"
#include "bufferSetup.h"
#include "textureLoader.h"
#include "shaderUniforms.h"
#include "input.h"

#include <iostream>
#include <vector>

// -------------------------------------------------------------------------
// Screen
// -------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// -------------------------------------------------------------------------
// Shader / asset paths
// -------------------------------------------------------------------------
const char* vertexShader[] = { "vPhongShader.vert", "vLightShader.vert" };
const char* fragmentShader[] = { "fPhongShader.frag", "fLightShader.frag" };
const char* wallFilePath = "platform.txt";

float deltaTime, lastFrame;

// -------------------------------------------------------------------------
// ImGui / menu state
// -------------------------------------------------------------------------
const char* const mouseStateArr[] = { "FREE", "TANK" };
MouseState mouseState = MouseState::FREE;

// -------------------------------------------------------------------------
// Camera
// ----------------------------------------------S---------------------------
glm::vec3 cameraPos = glm::vec3(1.0f, 1.0f, -3.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float     yaw = 90.0f;
float     pitch = 0.0f;

Camera    camera = Camera(cameraPos, cameraUp, yaw, pitch);

// -------------------------------------------------------------------------
// Matrices
// -------------------------------------------------------------------------
glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(1.2f, 1.0f, 2.0f)), glm::vec3(1.0f));
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

int main()
{
    // ------------------------------------------------------------------
    // GLFW – init and configure
    // ------------------------------------------------------------------
    std::cout << "Starting GLFW init..." << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // ------------------------------------------------------------------
    // GLFW – window creation
    // ------------------------------------------------------------------
    std::cout << "Creating window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    MouseState mouseState = FREE;
    AppState appState = AppState(mouseState, camera, projection);

    glfwMakeContextCurrent(window);
    appState.setCallbacks(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ------------------------------------------------------------------
    // GLAD – load OpenGL function pointers
    // ------------------------------------------------------------------
    std::cout << "Loading GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ------------------------------------------------------------------
    // Shaders
    // ------------------------------------------------------------------
    std::cout << "Compiling shaders..." << std::endl;
    Shader ourShader(vertexShader[0], fragmentShader[0]);
    Shader lightCubeShader(vertexShader[1], fragmentShader[1]);

    for (int i = 0; i < (int)std::size(vertexShader); i++)
        std::cout << "Vertex shader:   " << vertexShader[i] << std::endl;
    for (int i = 0; i < (int)std::size(fragmentShader); i++)
        std::cout << "Fragment shader: " << fragmentShader[i] << std::endl;

    // ------------------------------------------------------------------
    // GPU buffers
    // ------------------------------------------------------------------
    std::cout << "Uploading vertex data..." << std::endl;
    unsigned int VBO;
    VAOs vaos = setupBuffers(VBO);

    // ------------------------------------------------------------------
    // Textures
    // ------------------------------------------------------------------
    std::cout << "Loading textures..." << std::endl;
    unsigned int texture1, texture2;
    loadTexture("container2.png", texture1);
    loadTexture("container2.png", texture2);

    // ------------------------------------------------------------------
    // Shader uniforms (static / one-time)
    // ------------------------------------------------------------------
    std::cout << "Setting uniforms..." << std::endl;

    ShaderUniform shaderUniforms = ShaderUniform(ourShader, texture1, texture2, model,camera, projection, material, light);
    shaderUniforms.initShaderUniforms();
    // ------------------------------------------------------------------
    // ImGui
    // ------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ------------------------------------------------------------------
    // World
    // ------------------------------------------------------------------
    std::vector<std::vector<std::string>> floors = generateWorld(wallFilePath);
    glm::vec3 startPos = glm::vec3(0.0f);
    float     rotation = 0.0f;

    // ------------------------------------------------------------------
    // Render loop
    // ------------------------------------------------------------------
    std::cout << "Entering render loop..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        // Timing
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        appState.processInput(window, ourShader, deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderUniforms.updateFrameUniforms();
        camera.UpdateRotation(deltaTime);
        camera.UpdatePosition(deltaTime);

        glBindVertexArray(vaos.cube);
        renderWorld(ourShader, floors, startPos, rotation);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Once);
        ImGui::Begin("Controls");

        int selectedMouseState = (int)mouseState;
        if (ImGui::Combo("Mouse Mode", &selectedMouseState, mouseStateArr, IM_ARRAYSIZE(mouseStateArr)))
        {
            mouseState = (MouseState)selectedMouseState;
        }
        ImGui::InputFloat3("Camera Position", glm::value_ptr(camera.Position));
        ImGui::InputFloat3("Camera Direction", glm::value_ptr(camera.Front));
        ImGui::DragFloat3("Position", glm::value_ptr(startPos), 0.1f);
        ImGui::SliderFloat("Rotation", &rotation, 0.0f, 360.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ------------------------------------------------------------------
    // Cleanup
    // ------------------------------------------------------------------
    glDeleteVertexArrays(1, &vaos.cube);
    glDeleteVertexArrays(1, &vaos.light);
    glDeleteBuffers(1, &VBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}