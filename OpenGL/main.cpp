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
#include "input.h"
#include "world.h"
#include "materialRegistry.h"
#include "scripts.h"

#include <iostream>
#include <vector>
#include <GameObject.h>

// -------------------------------------------------------------------------
// Screen
// -------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

// -------------------------------------------------------------------------
// Shader / asset paths
// -------------------------------------------------------------------------
char worldPath[256] = "world.txt";

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

GlobalLightSettings globalLights = {
    0.03f,                  // ambientStrength
    glm::vec3(1.0f),        // ambientColor
    true,                   // dirLightEnabled
    true                    // pointLightsEnabled
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

    glfwMakeContextCurrent(window);
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
    // ImGui
    // ------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    AppState appState = AppState(mouseState, camera, projection);
    appState.setCallbacks(window);

    std::cout << "Loading textures..." << std::endl;

    unsigned int texture1 = 0, texture2 = 0;
    bool ok1 = loadTexture("container2.png", texture1);
    bool ok2 = loadTexture("container2_specular.png", texture2);
    if (!ok1 || !ok2) {
        std::cerr << "Texture load failed, continuing without textures." << std::endl;
    }

    std::vector<float> vertexData = std::vector<float>(std::begin(vertices), std::end(vertices));
    std::vector<int> cubeLayout = {3, 3, 2}; // pos, normal, uv
    Mesh cubeMesh(vertexData, cubeLayout);

    // seperate registry for world
    MaterialRegistry blockMaterials;
    blockMaterials.add(0, Material({ { texture1, "material.diffuse"  }, { texture2, "material.specular" } }));

    World world(16, 16, 16);
    world.platform(16, 16);
    appState.setWorld(world);

    Registry registry;

    Shader litShader("lit.vert", "lit.frag");


    TransformComponent rootTransform{
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
    };
    int root = spawnEntity(registry, "root", rootTransform, /* parent */ NULL_ENTITY);

    TransformComponent solarSystemTransform{
    .position = glm::vec3(13.0f, 2.0f, 3.0f),
    };
    int solarSystem = spawnEntity(registry, "solarSystem", solarSystemTransform, root);


    TransformComponent sunTransform{
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
    };
    int sun = spawnEntity(registry, "sun", sunTransform, /* parent */ solarSystem);

    MaterialComponent sunMaterial {
        .color = glm::vec3(1.0f),
        .diffuseTexture = texture1,
        .specularTexture = texture2,
    };
    addMesh(registry, sun, &cubeMesh, &litShader, sunMaterial);
    addScript(registry, sun, /* updateFn */ &spinBehavior);


    TransformComponent childCubeTransform{
        .position = glm::vec3(2.0f, 0.0f, 0.0f),
        .scale = glm::vec3(0.5f, 0.5f, 0.5f),
    };
    int childCube = spawnEntity(registry, "childCube", childCubeTransform, /* parent */ solarSystem);

    MaterialComponent childCubeMaterial{
        .color = glm::vec3(0.2f, 0.4f, 0.9f),
        .shininess = 16.0f,
    };
    addMesh(registry, childCube, &cubeMesh, &litShader, childCubeMaterial);


    TransformComponent lampTransform{
        .position = glm::vec3(3.0f, 4.0f, 3.0f),
    };
    int lamp = spawnEntity(registry, "lamp", lampTransform, /* parent */ root);

    MaterialComponent lampMaterial{
        .color = glm::vec3(1.0f, 1.0f, 1.0f),
        .shininess = 32.0f,
        .emissive = 1.0f,
    };
    addMesh(registry, lamp, &cubeMesh, &litShader, lampMaterial);

    PointLightComponent lampLight{
        .color = glm::vec3(1.0f, 0.95f, 0.85f),
        .intensity = 2.5f,
    };
    addPointLight(registry, lamp, lampLight);


    TransformComponent dirLightTransform{
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
    };
    int dirLight = spawnEntity(registry, "dirLight", dirLightTransform, /* parent */ root);

    DirLightComponent globalLight{
        .direction = glm::vec3(-0.3f, -1.0f, -0.2f),
        .color = glm::vec3(1.0f, 0.98f, 0.9f),
        .intensity = 1.2f,
    };
    addDirLight(registry, dirLight, globalLight);



    // ------------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------------
    std::cout << "Entering render loop..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        appState.processInput(window, litShader, deltaTime);

        camera.UpdateRotation(deltaTime);
        camera.UpdatePosition(deltaTime);

        glm::mat4 view = camera.GetViewMatrix();

        glBindVertexArray(cubeMesh.getVAO());
        world.draw(litShader, blockMaterials);

        scriptSystem(registry, currentFrame, deltaTime);
        transformSystem(registry, root, glm::mat4(1.0f));

        renderSystem(registry, view, projection, camera.Position, globalLights);


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

        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::SliderFloat("Ambient Strength", &globalLights.ambientStrength, 0.0f, 1.0f);
            ImGui::ColorEdit3("Ambient Color", glm::value_ptr(globalLights.ambientColor));

            ImGui::Checkbox("Directional Light Enabled", &globalLights.dirLightEnabled);
            ImGui::Checkbox("Point Lights Enabled", &globalLights.pointLightsEnabled);

            ImGui::Separator();
            ImGui::Text("Sun");
            if (registry.dirLights.count(dirLight))
            {
                auto& globalLight = registry.dirLights[dirLight];
                if (ImGui::SliderFloat3("Sun Direction", glm::value_ptr(globalLight.direction), -1.0f, 1.0f))
                {
                    if (glm::length(globalLight.direction) > 0.0001f)
                        globalLight.direction = glm::normalize(globalLight.direction);
                }
                ImGui::ColorEdit3("Sun Color", glm::value_ptr(globalLight.color));
                ImGui::SliderFloat("Sun Intensity", &globalLight.intensity, 0.0f, 5.0f);
            }

            ImGui::Separator();
            ImGui::Text("Lamp");
            if (registry.pointLights.count(lamp))
            {
                auto& lampLight = registry.pointLights[lamp];
                ImGui::ColorEdit3("Lamp Color", glm::value_ptr(lampLight.color));
                ImGui::SliderFloat("Lamp Intensity", &lampLight.intensity, 0.0f, 10.0f);
                ImGui::SliderFloat("Lamp Linear", &lampLight.linear, 0.0f, 1.0f);
                ImGui::SliderFloat("Lamp Quadratic", &lampLight.quadratic, 0.0f, 2.0f);
            }
            if (registry.materials.count(lamp))
            {
                auto& lampMat = registry.materials[lamp];
                ImGui::SliderFloat("Lamp Emissive", &lampMat.emissive, 0.0f, 1.0f);
            }
        }

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // ------------------------------------------------------------------
    // Cleanup
    // ------------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}