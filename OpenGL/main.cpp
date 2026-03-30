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

glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

// positions all containers
glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

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

    glEnable(GL_DEPTH_TEST);

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
    loadTexture("container2_specular.png", texture2);

    // ------------------------------------------------------------------
    // Shader uniforms (static / one-time)
    // ------------------------------------------------------------------
    std::cout << "Setting uniforms..." << std::endl;

    OurShaderUniform ourShaderUniforms = OurShaderUniform(ourShader, texture1, texture2, model,camera, projection, material, light, pointLightPositions);
    LightCubeShaderUniform lightCubeShaderUniform = LightCubeShaderUniform(lightCubeShader, model, camera, projection);
    ourShaderUniforms.initShaderUniforms();
    lightCubeShaderUniform.initShaderUniforms();
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

        camera.UpdateRotation(deltaTime);
        camera.UpdatePosition(deltaTime);

        ourShaderUniforms.updateFrameUniforms();
        lightCubeShaderUniform.updateFrameUniforms();

        ourShader.use();
        glBindVertexArray(vaos.cube);
        for(int i = 0 ; i < sizeof(cubePositions) / sizeof(cubePositions[0]); i++) 
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20 * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            ourShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        lightCubeShader.use();
        glBindVertexArray(vaos.light);
        for (int i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightCubeShader.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

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