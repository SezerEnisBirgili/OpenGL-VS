#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <assimp/defs.h>
#include <assimp/version.h>

#include "engine.h"
#include "settings.h"
#include "vertexData.h"
#include "input.h"
#include "GameObject.h"
#include "world.h"
#include "player.h"
#include "scripts.h"
#include "AssimpImporter.h"

#include <iostream>
#include <string>
#include <sys/types.h>
#include <vector>

char worldPath[256] = "world.txt";

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// -------------------------------------------------------------------------
// ImGui / menu state
// -------------------------------------------------------------------------
const char* const mouseStateArr[] = { "FREE", "TANK" };
MouseState mouseState = MouseState::FREE;

struct BlockPaletteEntry {
    int entity;
    std::string name;
    unsigned int diffuseTexture;
};

std::vector<BlockPaletteEntry> blockPalette;

std::vector<Vertex> floatArrayToVertexVector(const float* data, int count);

int main();

GLFWwindow* initOpenGL();
void initImGui(GLFWwindow* window);
void buildBlockPalette(Registry& reg, std::vector<BlockPaletteEntry>& out);
void renderImGui(Registry& reg, Player& player, int e, int dirLight, int lamp, const std::vector<BlockPaletteEntry>& palette);

int setupSceneGraph(Registry& registry, int litShader,
    int sunMeshId, int earthMeshId, int moonMeshId, int lampMeshId, int grassMeshId, int cubeMeshId,
    unsigned int texSun, unsigned int texWorld, unsigned int texMoon,
    unsigned int fallbackDiffuse, unsigned int fallbackSpecular,
    unsigned int grassTex, unsigned int whiteTex,
    int& outDirLight, int& outLamp);

int main()
{
    GLFWwindow* window = initOpenGL();
    if (!window) return -1;

    initImGui(window);

    Engine engine(engineSettings);
    Player& player = engine.getPlayer();
    Camera defaultCam(glm::vec3(1.0f, 1.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);
    player.setCamera(defaultCam);
    Registry& registry = engine.getRegistry();

    engine.getInputManager().setupCallbacks(window);

    std::cout << "Loading textures..." << std::endl;

    //-REQUIRED-//////////////////////////////////////////////////////////////////////////////////
    unsigned int fallbackDiffuse       = loadTexture(registry, "missing_texture.png",     0, true);
    unsigned int fallbackSpecular      = loadTexture(registry, "missing_specular.png",    0, true);
    //////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int texContainer2         = loadTexture(registry, "container2.png",          1, true);
    unsigned int texContainer2Specular = loadTexture(registry, "container2_specular.png", 1, true);
    unsigned int texWorld              = loadTexture(registry, "world.png",               1, true);
    unsigned int texSun                = loadTexture(registry, "sun.png",                 1, true);
    unsigned int texMoon               = loadTexture(registry, "moon.png",                1, true);
    unsigned int grass                 = loadTexture(registry, "grass.png",               1, true);
    unsigned int white                 = loadTexture(registry, "white.png",               1, true);

    int const COLORED_BLOCK_SIZE = 16;
    int coloredBlockTextures[COLORED_BLOCK_SIZE];
    int coloredBlocksMesh[COLORED_BLOCK_SIZE];
    std::vector<std::string> colorNames = {"white", "orange", "magenta", "light_blue", "yellow", "lime", "pink", "gray", "light_gray", "cyan", "purple", "blue", "brown", "green", "red", "black"};

    for(int i = 0; i < COLORED_BLOCK_SIZE; i++) {
        coloredBlockTextures[i] = loadTexture(registry, colorNames[i] + ".png", 1, true);
    }

    if (!fallbackDiffuse || !fallbackSpecular || !texContainer2 || !texContainer2Specular || !texWorld || !texSun || !texMoon) {
        std::cerr << "One or more textures failed to load, continuing with fallbacks." << std::endl;
    }

    int litShader     = registry.registerShader       ("lit.vert",     "lit.frag");
    int worldShader   = registry.registerShader       ("world.vert",   "lit.frag");
    int outlineShader = registry.registerOutlineShader("outline.vert", "outline.frag");

    std::vector<Vertex> squareVerts = floatArrayToVertexVector(basicSquareVertices,             std::size(basicSquareVertices));
    std::vector<Vertex> cubeVerts   = floatArrayToVertexVector(basicCubeVertices,               std::size(basicCubeVertices));
    std::vector<Vertex> earthVerts  = floatArrayToVertexVector(basicCubeWrappedTextureVertices, std::size(basicCubeWrappedTextureVertices));

    // auto generate indices for compatbility, inefficient
    std::vector<unsigned int> squareIndices (std::begin(::squareIndices), std::end(::squareIndices));
    std::vector<unsigned int> cubeIndices   (std::begin(::cubeIndices),   std::end(::cubeIndices));

    int cubeMesh     = registry.registerMesh(cubeVerts,   cubeIndices,   "cube");

    int sunMesh      = registry.registerMesh(cubeVerts,   cubeIndices,   "cube");
    int earthMesh    = registry.registerMesh(earthVerts,  cubeIndices,   "earth");
    int moonMesh     = registry.registerMesh(cubeVerts,   cubeIndices,   "cube");

    int platformMesh = registry.registerMesh(cubeVerts,   cubeIndices,   "cube");
    int lampMesh     = registry.registerMesh(cubeVerts,   cubeIndices,   "cube");
    int grassMesh    = registry.registerMesh(squareVerts, squareIndices, "square");

    for(int i = 0; i < COLORED_BLOCK_SIZE; i++) {
        coloredBlocksMesh[i] = registry.registerMesh(cubeVerts, cubeIndices, "cube");
    }

    // ------------------------------------------------------------------
    // Scene graph
    // ------------------------------------------------------------------
    int dirLight = NULL_ENTITY;
    int lamp = NULL_ENTITY;
    int root = setupSceneGraph(registry, litShader,
        sunMesh, earthMesh, moonMesh, lampMesh, grassMesh, cubeMesh,
        texSun, texWorld, texMoon,
        fallbackDiffuse, fallbackSpecular,
        grass, white,
        dirLight, lamp);
    

    // .mesh vs .renderable : .mesh doesnt add to renderables list
    // .block: like .renderable, but also registers this entity in the block palette
    int redGlassBlock = EntityBuilder::create(registry, "Red Glass", glm::vec3(7.0f, 2.0f, 3.0f), glm::vec3(1.0f), root)
        .block(cubeMesh, litShader, {
            .isTransparent = true,
            .diffuseTexture = (int)white,
            .specularTexture = (int)fallbackSpecular,
            .color = { 1.0f, 0.0f, 0.0f },
            .alpha = 0.5f
            });
        
    int coloredBlockIds[COLORED_BLOCK_SIZE];
    for(int i = 0; i < COLORED_BLOCK_SIZE; i++) {
        coloredBlockIds[i] = EntityBuilder::create(registry, colorNames[i] + " Block", glm::vec3(0.0f), glm::vec3(1.0f), root)
        .block(cubeMesh, litShader, {
            .isTransparent = false,
            .diffuseTexture = coloredBlockTextures[i],
            .specularTexture = (int)fallbackSpecular,
            });

    }
    
    int world = WorldBuilder::create(registry, "world", 64, 64, 64, worldShader, outlineShader)
        .platform(coloredBlockIds[0] /*white block*/, 64, 64)
        .outlineColor(engineSettings.outlineColor);

    player.setWorld(registry.getWorld(world));
    std::cout << "main: player @ " << &player << " world set to " << &world << std::endl;

    buildBlockPalette(registry, blockPalette);
    if (!blockPalette.empty()) {
        player.changeSelectedBlock(blockPalette.front().entity);
    }


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

        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        engine.getInputManager().processInput(window, deltaTime);

        Camera& camera = player.getCamera();
        camera.UpdateRotation(deltaTime);
        camera.UpdatePosition(deltaTime);

        glm::vec3 hitBlockPos;
        player.lookingAtBlock(hitBlockPos);

        engine.update(currentFrame, deltaTime);
        engine.render();

        renderImGui(registry, player, world, dirLight, lamp, blockPalette);

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
    GLFWwindow* window = glfwCreateWindow(engineSettings.screenWidth, engineSettings.screenHeight, "LearnOpenGL", NULL, NULL);
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
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    io.DisplayFramebufferScale = ImVec2((float)fbWidth / engineSettings.screenWidth, (float)fbHeight / engineSettings.screenHeight);

    glViewport(0, 0, fbWidth, fbHeight);
}

std::vector<Vertex> floatArrayToVertexVector(const float* data, int count)
{
    std::vector<Vertex> vertices;
    int vertCount = count / 8;

    for (int i = 0; i < vertCount; i++)
    {
        Vertex v{};
        const float* base = data + i * 8;
        v.Position = { base[0], base[1], base[2] };
        v.Normal = { base[3], base[4], base[5] };
        v.TexCoords = { base[6], base[7] };
        vertices.push_back(v);
    }
    return vertices;
}

void buildBlockPalette(Registry& reg, std::vector<BlockPaletteEntry>& out)
{
    out.clear();

    for (int e : reg.blockPalette) {
        auto matIt = reg.materials.find(e);
        if (matIt == reg.materials.end()) continue;
        if (matIt->second.isAir) continue; // skip air

        std::string name = reg.names.count(e) ? reg.names.at(e) : ("entity_" + std::to_string(e));

        out.push_back({ e, name, (unsigned int)matIt->second.diffuseTexture });
    }

    std::cout << "[buildBlockPalette] found " << out.size() << " block entities" << std::endl;
}

void renderImGui(Registry& reg, Player& player, int e, int dirLight, int lamp, const std::vector<BlockPaletteEntry>& palette)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    World& world = *reg.getWorld(e);

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

    Camera& camera = player.getCamera();

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
        world.importWorldFromPath(reg, worldPath);
        player.setWorld(&world);
        std::cout << "World imported from " << worldPath << std::endl;
    }

    if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::SliderFloat("Ambient Strength", &lightSettings.ambientStrength, 0.0f, 1.0f);
        ImGui::ColorEdit3("Ambient Color", glm::value_ptr(lightSettings.ambientColor));

        ImGui::Checkbox("Directional Light Enabled", &lightSettings.dirLightEnabled);
        ImGui::Checkbox("Point Lights Enabled", &lightSettings.pointLightsEnabled);

        ImGui::Separator();
        ImGui::Text("Sun");
        if (reg.dirLights.count(dirLight))
        {
            auto& globalLight = reg.dirLights[dirLight];
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
        if (reg.pointLights.count(lamp))
        {
            auto& lampLight = reg.pointLights[lamp];
            ImGui::ColorEdit3("Lamp Color", glm::value_ptr(lampLight.color));
            ImGui::SliderFloat("Lamp Intensity", &lampLight.intensity, 0.0f, 10.0f);
            ImGui::SliderFloat("Lamp Linear", &lampLight.linear, 0.0f, 1.0f);
            ImGui::SliderFloat("Lamp Quadratic", &lampLight.quadratic, 0.0f, 2.0f);
        }
        if (reg.materials.count(lamp))
        {
            auto& lampMat = reg.materials[lamp];
            ImGui::SliderFloat("Lamp Emissive", &lampMat.emissive, 0.0f, 1.0f);
        }
    }

    if (ImGui::CollapsingHeader("World", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::ColorEdit3("Outline Color", glm::value_ptr(engineSettings.outlineColor)))
        {
            world.setOutlineColor(engineSettings.outlineColor);
        }
    }

    if (ImGui::CollapsingHeader("Block Palette", ImGuiTreeNodeFlags_DefaultOpen))
    {
        int currentBlock = player.getBlockToPlace();

        for (const auto& entry : palette)
        {
            ImGui::PushID(entry.entity);

            if (entry.diffuseTexture != 0)
            {
                ImGui::Image((ImTextureID)(intptr_t)entry.diffuseTexture, ImVec2(32, 32));
                ImGui::SameLine();
            }

            bool isSelected = (entry.entity == currentBlock);
            if (ImGui::Selectable(entry.name.c_str(), isSelected))
            {
                player.changeSelectedBlock(entry.entity);
            }

            ImGui::PopID();
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// -------------------------------------------------------------------------
// Scene Graph Creation Helpers
// -------------------------------------------------------------------------

void setupSolarSystem(Registry& registry, 
    int litShader, int parentEntity,
    int sunMeshId, int earthMeshId, int moonMeshId,
    unsigned int texSun, unsigned int texWorld, unsigned int texMoon,
    unsigned int fallbackSpecular)
{
    int solarSystem = EntityBuilder::create(registry, "solarSystem", { 20.0f, 3.0f, 20.0f }, glm::vec3(1.0f), parentEntity);

    EntityBuilder::create(registry, "sun", { 0.0f, 0.0f, 0.0f }, glm::vec3(1.0f), solarSystem)
        .renderable(sunMeshId, litShader, {
            .diffuseTexture = (int)texSun, .specularTexture = (int)fallbackSpecular,
            .color = glm::vec3(1.0f), .emissive = 1.0f
            })
        .script(spinBehavior(0.2f))
        .pointLight({ .color = glm::vec3(1.0f), .intensity = 1.0f });

    int earth = EntityBuilder::create(registry, "earth", { 3.0f, 0.0f, 0.0f }, glm::vec3(0.5f), solarSystem)
        .renderable(earthMeshId, litShader, {
            .diffuseTexture = (int)texWorld, .specularTexture = (int)texWorld,
            .color = {0.2f, 0.4f, 0.9f}, .shininess = 16.0f
            })
        .script(earthOrbitBehavior(0.25f, 0.25f, 23.5f));

    EntityBuilder::create(registry, "moon", { 1.5f, 0.0f, 0.0f }, glm::vec3(0.5f), earth)
        .renderable(moonMeshId, litShader, {
            .diffuseTexture = (int)texMoon, .specularTexture = (int)texMoon,
            .color = {0.2f, 0.4f, 0.9f}, .shininess = 0.0f
            })
        .script(moonBehavior(1.0f));
}

void setupLights(Registry& registry, int litShader, int parentEntity, int lampMeshId, unsigned int fallbackDiffuse, unsigned int fallbackSpecular, int& outDirLight, int& outLamp)
{
    outLamp = EntityBuilder::create(registry, "lamp", glm::vec3(10.0f, 5.0f, 10.0f), glm::vec3(1.0f), parentEntity)
        .renderable(lampMeshId, litShader, {
            .diffuseTexture = (int)fallbackDiffuse, .specularTexture = (int)fallbackSpecular,
            .color = glm::vec3(1.0f), .shininess = 32.0f, .emissive = 1.0f
            })
        .pointLight({ .color = {1.0f, 0.95f, 0.85f}, .intensity = 1.0f });

    outDirLight = EntityBuilder::create(registry, "dirLight", glm::vec3(0.0f), glm::vec3(1.0f), parentEntity)
    .dirLight({ .direction = glm::normalize(glm::vec3(0.5f, -1.5f, -0.8f)), .color = {1.0f, 0.98f, 0.9f}, .intensity = 1.0f });
}

void setupVegetation(Registry& registry, int litShader, int parentEntity, int grassMeshId, unsigned int grassTex, unsigned int fallbackSpecular)
{
    const glm::vec3 positions[] = {
        {3.1f, 2.0f, 3.1f}, {5.1f, 2.0f, 3.1f}, {3.1f, 2.0f, 5.1f}, {5.1f, 2.1f, 5.1f}
    };

    for (size_t i = 0; i < std::size(positions); ++i) {
        std::string name = "grass" + std::to_string(i + 1);
        EntityBuilder::create(registry, name, positions[i], glm::vec3(2.0f), parentEntity)
            .renderable(grassMeshId, litShader, {
                .diffuseTexture = (int)grassTex, .specularTexture = (int)fallbackSpecular,
                .isMasked = true
                });
    }
}

int setupSceneGraph(Registry& registry, int litShader,
    int sunMeshId, int earthMeshId, int moonMeshId, int lampMeshId, int grassMeshId, int cubeMeshId,
    unsigned int texSun, unsigned int texWorld, unsigned int texMoon,
    unsigned int fallbackDiffuse, unsigned int fallbackSpecular,
    unsigned int grassTex, unsigned int whiteTex,
    int& outDirLight, int& outLamp)
{
    int root = EntityBuilder::create(registry, "root");

    AssimpImporter importer(registry, "backpack/backpack.obj", litShader);
    importer.loadModel("backpack", { .position = {0.0f, 5.0f, 0.0f} }, root);

    setupSolarSystem(registry, litShader, root, sunMeshId, earthMeshId, moonMeshId, texSun, texWorld, texMoon, fallbackSpecular);
    setupLights(registry, litShader, root, lampMeshId, fallbackDiffuse, fallbackSpecular, outDirLight, outLamp);
    setupVegetation(registry, litShader, root, grassMeshId, grassTex, fallbackSpecular);

    return root;
}