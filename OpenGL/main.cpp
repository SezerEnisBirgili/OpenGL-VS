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

void initWorld(World& world, int sizeX, int sizeZ)
{
    for (int x = 0; x < sizeX; x++)
        for (int z = 0; z < sizeZ; z++)
            world.setBlock(x, 0, z, true, 0); // y=0 layer, texture id 0
}

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

    AppState appState = AppState(mouseState, camera, projection);

    glfwMakeContextCurrent(window);
    appState.setCallbacks(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // because vm
    //if (glfwRawMouseMotionSupported())
    //    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    // ------------------------------------------------------------------
    // GLAD – load OpenGL function pointers
    // ------------------------------------------------------------------
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
    // Textures
    // ------------------------------------------------------------------
    std::cout << "Loading textures..." << std::endl;
    unsigned int texture1, texture2;
    loadTexture("container2.png", texture1);
    loadTexture("container2_specular.png", texture2);

    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);
    // ------------------------------------------------------------------
    // Shader uniforms (static / one-time)
    // ------------------------------------------------------------------
    std::cout << "Setting uniforms..." << std::endl;

    OurShaderUniform       ourShaderUniforms      = OurShaderUniform(ourShader, camera, projection, pointLightPositions);
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
    initWorld(world, 16, 16);
    appState.setWorld(world);

    // ------------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------------
    std::cout << "Entering render loop..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        // ------------------------------------------------------------------
        // Timing
        // ------------------------------------------------------------------
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ------------------------------------------------------------------
        // Input
        // ------------------------------------------------------------------
        appState.processInput(window, ourShader, deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.UpdateRotation(deltaTime);
        camera.UpdatePosition(deltaTime);

        // ------------------------------------------------------------------
        // Update shader uniforms every frame (view/projection/lighting)
        // ------------------------------------------------------------------
        ourShaderUniforms.updateFrameUniforms();
        lightCubeShaderUniform.updateFrameUniforms();

        // ------------------------------------------------------------------
        // Render loop
        // ------------------------------------------------------------------

        ourShader.use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(vaos.cube);

        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 16; y++)
            {
                for (int z = 0; z < 16; z++)
                {
                    if (!world.isBlockSolid(x, y, z))
                        continue;

                    glm::mat4 blockModel = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
                    ourShader.setMat4("model", blockModel);

                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }

        // --- Point light cubes ---
        lightCubeShader.use();
        glBindVertexArray(vaos.light);
        for (int i = 0; i < sizeof(pointLightPositions) / sizeof(pointLightPositions[0]); i++)
        {
            glm::mat4 lightModel = glm::mat4(1.0f);
            lightModel = glm::translate(lightModel, pointLightPositions[i]);
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
            if (exportWorldToPath(world, worldPath))
                std::cout << "World exported to " << worldPath << std::endl;
            else
                std::cout << "World export failed!" << std::endl;
        }

        ImGui::SameLine();

        if (ImGui::Button("Import World"))
        {
            World loaded = importWorldFromPath(worldPath);
            world = loaded;
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