#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <assimp/defs.h>
#include <assimp/version.h>

#include "shader.h"
#include "vertexData.h"
#include "mesh.h"
#include "input.h"
#include "world.h"
#include "scripts.h"
#include "Importer.h"

#include <iostream>
#include <vector>
#include <GameObject.h>

// -------------------------------------------------------------------------
// Screen
// -------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 1024;

glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

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

std::vector<Vertex> floatArrayToVertexVector(const float* data, size_t count);

GLFWwindow* initOpenGL();
void initImGui(GLFWwindow* window);
void renderImGui(Registry& registry, World& world, AppState& appState, int dirLight, int lamp);

// Scene Graph Helpers
void setupSolarSystem(Registry& registry, Shader& litShader, int parentEntity, const Mesh& sunMesh, const Mesh& earthMesh, const Mesh& moonMesh);
void setupLights     (Registry& registry, Shader& litShader, int parentEntity, const Mesh& lampMesh, int& outDirLight, int& outLamp);
int  setupSceneGraph (Registry& registry, Shader& litShader, const Mesh& sunMesh, const Mesh& earthMesh,  const Mesh& moonMesh, const Mesh& lampMesh, int& outDirLight, int& outLamp);

int main()
{

    GLFWwindow* window = initOpenGL();
    if (!window) return -1;

    initImGui(window);

    Registry registry;

    AppState appState = AppState(mouseState, camera, projection);
    appState.setCallbacks(window);

    std::cout << "Loading textures..." << std::endl;

    unsigned int fallbackDiffuse       = registry.loadTexture("missing_texture.png",     1, true, "missing_texture");
    unsigned int fallbackSpecular      = registry.loadTexture("missing_specular.png",    1, true, "missing_specular");
    unsigned int texContainer2         = registry.loadTexture("container2.png",          1, true, "container2");
    unsigned int texContainer2Specular = registry.loadTexture("container2_specular.png", 1, true, "container2_specular");
    unsigned int texWorld              = registry.loadTexture("world.png",               1, true, "world");
    unsigned int texSun                = registry.loadTexture("sun.png",                 1, true, "sun");
    unsigned int texMoon               = registry.loadTexture("moon.png",                1, true, "moon");

    if (!fallbackDiffuse || !fallbackSpecular || !texContainer2 || !texContainer2Specular || !texWorld || !texSun || !texMoon) {
        std::cerr << "One or more textures failed to load, continuing with fallbacks." << std::endl;
    }

    // separate registry for world
    MaterialRegistry blockMaterials;
    blockMaterials.add(0, Material({
        { texContainer2,         "material.diffuse"  },
        { texContainer2Specular, "material.specular" }}));


    Shader litShader("lit.vert", "lit.frag");


    std::vector<Vertex> cubeVerts  = floatArrayToVertexVector(basicCubeVertices,               std::size(basicCubeVertices));
    std::vector<Vertex> earthVerts = floatArrayToVertexVector(basicCubeWrappedTextureVertices, std::size(basicCubeWrappedTextureVertices));
    
    std::vector<unsigned int> cubeIndices(std::begin(::cubeIndices), std::end(::cubeIndices));
    
    
    // if emmisive no specular map
    Mesh sunMesh  (cubeVerts,  cubeIndices, texSun,          0);
    Mesh earthMesh(earthVerts, cubeIndices, texWorld,        texWorld);
    Mesh moonMesh (cubeVerts,  cubeIndices, texMoon,         texMoon);
    Mesh lampMesh (cubeVerts,  cubeIndices, fallbackDiffuse, 0);


    World world(16, 16, 16);
    world.initMesh(cubeVerts, cubeIndices); 
    world.platform(16, 16);
    appState.setWorld(world);

    // ------------------------------------------------------------------
    // Scene graph
    // ------------------------------------------------------------------
    int dirLight = NULL_ENTITY;
    int lamp = NULL_ENTITY;

    int root = setupSceneGraph(registry, litShader, sunMesh, earthMesh, moonMesh, lampMesh, dirLight, lamp);

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

        world.draw(litShader, blockMaterials);

        scriptSystem(registry, currentFrame, deltaTime);
        transformSystem(registry, root, glm::mat4(1.0f));

        renderSystem(registry, view, projection, camera.Position, globalLights);

        renderImGui(registry, world, appState, dirLight, lamp);

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}


GLFWwindow* initOpenGL()
{
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
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    std::cout << "Loading GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    const char* glVersion = (const char*)glGetString(GL_VERSION);
    const char* glRenderer = (const char*)glGetString(GL_RENDERER);
    std::cout << "OpenGL version: " << (glVersion ? glVersion : "NULL") << std::endl;
    std::cout << "Renderer:       " << (glRenderer ? glRenderer : "NULL") << std::endl;

    std::cout << "--- Assimp Runtime Binary ---" << std::endl;
    std::cout << "Major Version: " << aiGetVersionMajor() << std::endl;
    std::cout << "Minor Version: " << aiGetVersionMinor() << std::endl;
    std::cout << "Revision:      " << aiGetVersionRevision() << std::endl;

    glEnable(GL_DEPTH_TEST);

    return window;
}

void initImGui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    io.DisplayFramebufferScale = ImVec2((float)fbWidth / SCR_WIDTH, (float)fbHeight / SCR_HEIGHT);

    glViewport(0, 0, fbWidth, fbHeight);
}

std::vector<Vertex> floatArrayToVertexVector(const float* data, size_t count)
{
    std::vector<Vertex> vertices;

    // each vertex is 8 floats: pos(3) + normal(3) + uv(2)
    int vertCount = count / 8;
    
    for (int i = 0; i < vertCount; i++)
    {
        Vertex v{};
        const float* base = data + i * 8;
        v.Position  = { base[0], base[1], base[2] };
        v.Normal    = { base[3], base[4], base[5] };
        v.TexCoords = { base[6], base[7] };
        vertices.push_back(v);
    }
    return vertices;
}

void renderImGui(Registry& registry, World& world, AppState& appState, int dirLight, int lamp)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
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
}

// -------------------------------------------------------------------------
// Scene Graph Creation Helpers
// -------------------------------------------------------------------------

void setupSolarSystem(Registry& registry, Shader& litShader, int parentEntity, const Mesh& sunMesh, const Mesh& earthMesh, const Mesh& moonMesh)
{
    TransformComponent solarSystemTransform{
        .position = glm::vec3(13.0f, 2.0f, 3.0f),
    };
    int solarSystem = spawnEntity(registry, "solarSystem", solarSystemTransform, parentEntity);

    // Sun
    TransformComponent sunTransform{
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
    };
    int sun = spawnEntity(registry, "sun", sunTransform, solarSystem);

    MaterialComponent sunMaterial{
        .color = glm::vec3(1.0f),
        .emissive = 1.0f,
    };
    addMesh(registry, sun, sunMesh, &litShader, sunMaterial);
    addScript(registry, sun, spinBehavior(0.2f));

    PointLightComponent sunLight{
        .color = glm::vec3(1.0f, 1.0f, 1.0f),
        .intensity = 1.0f,
    };
    addPointLight(registry, sun, sunLight);

    // Earth
    TransformComponent earthTransform{
        .position = glm::vec3(3.0f, 0.0f, 0.0f),
        .scale = glm::vec3(0.5f, 0.5f, 0.5f),
    };
    int earth = spawnEntity(registry, "earth", earthTransform, solarSystem);

    MaterialComponent earthMaterial{
        .color = glm::vec3(0.2f, 0.4f, 0.9f),
        .shininess = 16.0f,
    };
    addMesh(registry, earth, earthMesh, &litShader, earthMaterial);
    addScript(registry, earth, earthOrbitBehavior(0.25f, 0.25f, 23.5f));

    // Moon
    TransformComponent moonTransform{
        .position = glm::vec3(1.5f, 0.0f, 0.0f),
        .scale = glm::vec3(0.5f, 0.5f, 0.5f),
    };
    int moon = spawnEntity(registry, "moon", moonTransform, earth);

    MaterialComponent moonMaterial{
        .color = glm::vec3(0.2f, 0.4f, 0.9f),
        .shininess = 0.0f,
    };
    addMesh(registry, moon, moonMesh, &litShader, moonMaterial);
    addScript(registry, moon, moonBehavior(1.0f));
}

void setupLights(Registry& registry, Shader& litShader, int parentEntity, const Mesh& lampMesh, int& outDirLight, int& outLamp)
{
    // Point Light Lamp
    TransformComponent lampTransform{
        .position = glm::vec3(3.0f, 4.0f, 3.0f),
    };
    outLamp = spawnEntity(registry, "lamp", lampTransform, parentEntity);

    MaterialComponent lampMaterial{
        .color = glm::vec3(1.0f, 1.0f, 1.0f),
        .shininess = 32.0f,
        .emissive = 1.0f,
    };
    addMesh(registry, outLamp, lampMesh, &litShader, lampMaterial);

    PointLightComponent lampLight{
        .color = glm::vec3(1.0f, 0.95f, 0.85f),
        .intensity = 2.5f,
    };
    addPointLight(registry, outLamp, lampLight);

    // Directional Light
    TransformComponent dirLightTransform{
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
    };
    outDirLight = spawnEntity(registry, "dirLight", dirLightTransform, parentEntity);

    DirLightComponent globalLight{
        .direction = glm::normalize(glm::vec3(1.0f, -1.0f, -1.0f)),
        .color = glm::vec3(1.0f, 0.98f, 0.9f),
        .intensity = 1.2f,
    };
    addDirLight(registry, outDirLight, globalLight);
}

int setupSceneGraph(Registry& registry, Shader& litShader,  const Mesh& sunMesh, const Mesh& earthMesh, const Mesh& moonMesh, const Mesh& lampMesh, int& outDirLight, int& outLamp)
{
    TransformComponent rootTransform{
        .position = glm::vec3(0.0f, 0.0f, 0.0f),
    };
    int root = spawnEntity(registry, "root", rootTransform, /* parent */ NULL_ENTITY);

    AssimpImporter importer(registry, "/home/sezer/OpenGL-VS/OpenGL/backpack/backpack.obj", &litShader);
    TransformComponent backpackTransform{
        .position = glm::vec3(0.0f, 5.0f, 0.0f),
    };
    int model = importer.loadModel("backpack", backpackTransform, root);

    setupSolarSystem(registry, litShader, root, sunMesh, earthMesh, moonMesh);
    setupLights(registry, litShader, root, lampMesh, outDirLight, outLamp);

    return root;
}