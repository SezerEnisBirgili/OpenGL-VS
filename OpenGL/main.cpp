#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "shader.h"
#include "vertexData.h"
#include "bufferSetup.h"
#include "textureLoader.h"
#include "shaderUniforms.h"
#include "input.h"
#include "world.h"
#include "materialRegistry.h"

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
// -------------------------------------------------------------------------
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
    glm::vec3(2.0f,  2.0f,  2.0f),
    glm::vec3(4.0f,  2.0f,  4.0f),
    glm::vec3(6.0f,  2.0f, 12.0f),
    glm::vec3(8.0f,  2.0f,  1.0f)
};

int main()
{
    // ------------------------------------------------------------------
    // GLFW – init and configure
    // ------------------------------------------------------------------
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        });

    std::cout << "Starting GLFW init..." << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    std::cout << "Creating window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    AppState appState = AppState(mouseState, camera, projection);

    glfwMakeContextCurrent(window);
    appState.setCallbacks(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::cout << "Loading GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const char* glVersion = (const char*)glGetString(GL_VERSION);
    const char* glRenderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "OpenGL version: " << (glVersion ? glVersion : "NULL") << std::endl;
    std::cout << "Renderer:       " << (glRenderer ? glRenderer : "NULL") << std::endl;

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
    // Textures + Materials
    // ------------------------------------------------------------------
    std::cout << "Loading textures..." << std::endl;
    unsigned int texture1, texture2;
    loadTexture("container2.png", texture1);
    loadTexture("container2_specular.png", texture2);

    MaterialRegistry blockMaterials;
    blockMaterials.add(0, Material({ { texture1, "material.diffuse"  }, { texture2, "material.specular" }}));

    // ------------------------------------------------------------------
    // Shader uniforms (static / one-time)
    // ------------------------------------------------------------------
    std::cout << "Setting uniforms..." << std::endl;

    OurShaderUniform       ourShaderUniforms = OurShaderUniform(ourShader, camera, projection, pointLightPositions);
    LightCubeShaderUniform lightCubeShaderUniform = LightCubeShaderUniform(lightCubeShader, camera, projection);
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

    World world(16, 16, 16);
    world.platform(16, 16);
    appState.setWorld(world);

    // ------------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------------
    std::cout << "Entering render loop..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
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

        glBindVertexArray(vaos.cube);
        world.draw(ourShader, blockMaterials);

        // --- Point light cubes ---
        lightCubeShader.use();
        glBindVertexArray(vaos.light);
        for (int i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++)
        {
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel, pointLightPositions[i] - glm::vec3(0.1f));
            lightModel = glm::scale(lightModel, glm::vec3(0.2f));
            lightCubeShader.setMat4("model", lightModel);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // ------------------------------------------------------------------
        // IMGUI
        // ------------------------------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
           
        // --- Crosshair ---
        {
            ImVec2 center(SCR_WIDTH * 0.5f, SCR_HEIGHT * 0.5f);
            float size = 10.0f;
            float thickness = 2.0f;
            ImU32 color = IM_COL32(255, 255, 255, 220);

            ImDrawList* drawList = ImGui::GetForegroundDrawList();
            drawList->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size, center.y), color, thickness);
            drawList->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size), color, thickness);
        }

        ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Once);
        ImGui::Begin("Controls");

        int selectedMouseState = (int)mouseState;
        if (ImGui::Combo("Mouse Mode", &selectedMouseState, mouseStateArr, IM_ARRAYSIZE(mouseStateArr)))
        {
            mouseState = (MouseState)selectedMouseState;
        }
        ImGui::InputFloat3("Camera Position", glm::value_ptr(camera.Position));
        ImGui::InputFloat3("Camera Direction", glm::value_ptr(camera.Front));

        static char worldPath[256] = "world.txt";
        ImGui::InputText("World File", worldPath, IM_ARRAYSIZE(worldPath));

        if (ImGui::Button("Export World"))
        {
            if (world.exportWorldToPath(worldPath))
                std::cout << "World exported to " << worldPath << std::endl;
            else
                std::cout << "World export failed!" << std::endl;
        }

        ImGui::SameLine();

        if (ImGui::Button("Import World"))
        {
            world.importWorldFromPath(worldPath);
            appState.setWorld(world);
            std::cout << "World imported from " << worldPath << std::endl;
        }

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