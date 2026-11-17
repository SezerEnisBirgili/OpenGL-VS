// ---- External
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/version.h>

// ---- Project
#include "core/engine.h"
#include "core/settings.h"
#include "core/input.h"
#include "renderer/vertexData.h"
#include "entities/GameObject.h"
#include "world/world.h"
#include "entities/player.h"
#include "entities/scripts.h"
#include "io/AssimpImporter.h"
#include "ui/menu.h"

#include <iostream>
#include <string>
#include <vector>

// -------------------------------------------------------------------------
// Asset paths (ASSET_DIR is defined in CMakeLists.txt and ends with '/')
// -------------------------------------------------------------------------
const std::string TEXTURE_DIR = std::string(ASSET_DIR) + "textures/";
const std::string SHADER_DIR = std::string(ASSET_DIR) + "shaders/";
const std::string MODEL_DIR = std::string(ASSET_DIR) + "models/";
const std::string WORLD_DIR = std::string(ASSET_DIR) + "worlds/";

float deltaTime = 0.0f;
float lastFrame = 0.0f;

std::vector<Vertex> floatArrayToVertexVector(const float* data, int count);

int main();

GLFWwindow* initOpenGL();

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

    ImguiMenu ui;
    ui.init(window);

    Engine engine;
    Player& player = engine.getPlayer();
    Camera defaultCam(glm::vec3(1.0f, 1.0f, -3.0f), glm::vec3(0.0f, 1.0f, 0.0f), 90.0f, 0.0f);
    player.setCamera(defaultCam);
    Registry& registry = engine.getRegistry();

    engine.getInputManager().setupCallbacks(window);

    std::cout << "Loading textures..." << std::endl;

    //-REQUIRED-//////////////////////////////////////////////////////////////////////////////////
    unsigned int fallbackDiffuse = loadTexture(registry, TEXTURE_DIR + "fallback/missing_texture.png", 0, true);
    unsigned int fallbackSpecular = loadTexture(registry, TEXTURE_DIR + "fallback/missing_specular.png", 0, true);
    //////////////////////////////////////////////////////////////////////////////////////////////

    unsigned int texContainer2 = loadTexture(registry, TEXTURE_DIR + "containers/container2.png", 1, true);
    unsigned int texContainer2Specular = loadTexture(registry, TEXTURE_DIR + "containers/container2_specular.png", 1, true);
    unsigned int texLamp = loadTexture(registry, TEXTURE_DIR + "terrain/lamp.png", 0, true);
    unsigned int texWorld = loadTexture(registry, TEXTURE_DIR + "terrain/world.png", 0, true);
    unsigned int texSun = loadTexture(registry, TEXTURE_DIR + "terrain/sun.png", 0, true);
    unsigned int texMoon = loadTexture(registry, TEXTURE_DIR + "terrain/moon.png", 0, true);
    unsigned int grass = loadTexture(registry, TEXTURE_DIR + "terrain/grass.png", 1, true);
    unsigned int white = loadTexture(registry, TEXTURE_DIR + "colors/white.png", 0, true);

    int const COLORED_BLOCK_SIZE = 16;
    int coloredBlockTextures[COLORED_BLOCK_SIZE];
    int coloredBlocksMesh[COLORED_BLOCK_SIZE];
    std::vector<std::string> colorNames = { "white", "orange", "magenta", "light_blue", "yellow", "lime", "pink", "gray", "light_gray", "cyan", "purple", "blue", "brown", "green", "red", "black" };

    for (int i = 0; i < COLORED_BLOCK_SIZE; i++) {
        coloredBlockTextures[i] = loadTexture(registry, TEXTURE_DIR + "colors/" + colorNames[i] + ".png", 0, true);
    }

    if (!fallbackDiffuse || !fallbackSpecular || !texContainer2 || !texContainer2Specular || !texWorld || !texSun || !texMoon) {
        std::cerr << "One or more textures failed to load, continuing with fallbacks." << std::endl;
    }

    int litShader = registry.registerShader(SHADER_DIR + "lit.vert", SHADER_DIR + "lit.frag");
    int worldShader = registry.registerShader(SHADER_DIR + "world.vert", SHADER_DIR + "lit.frag");
    int outlineShader = registry.registerOutlineShader(SHADER_DIR + "outline.vert", SHADER_DIR + "outline.frag");

    AssimpImporter saver(registry, MODEL_DIR + "cube.obj", litShader);
    int basicCubeVerticesSize = (4 * 6 * 8);
    int VertexSize = 8;
    const unsigned int indexCount = (6 * 6);
    int indicesPerFace = 3;
    int status = saver.saveSimpleModel(MODEL_DIR + "cube.gltf", basicCubeVertices, basicCubeVerticesSize, VertexSize, cubeIndices, indexCount, indicesPerFace);

    if (status == 0) {
        std::cout << "Saved quad_test.gltf successfully!" << std::endl;
    }
    else {
        std::cerr << "Failed to save model." << std::endl;
    }

    std::vector<Vertex> squareVerts = floatArrayToVertexVector(basicSquareVertices, std::size(basicSquareVertices));
    std::vector<Vertex> cubeVerts = floatArrayToVertexVector(basicCubeVertices, std::size(basicCubeVertices));
    std::vector<Vertex> earthVerts = floatArrayToVertexVector(basicCubeWrappedTextureVertices, std::size(basicCubeWrappedTextureVertices));

    // auto generate indices for compatbility, inefficient
    std::vector<unsigned int> squareIndices(std::begin(::squareIndices), std::end(::squareIndices));
    std::vector<unsigned int> cubeIndices(std::begin(::cubeIndices), std::end(::cubeIndices));

    int cubeMesh = registry.registerMesh(cubeVerts, cubeIndices, "cube");

    int sunMesh = registry.registerMesh(cubeVerts, cubeIndices, "cube");
    int earthMesh = registry.registerMesh(earthVerts, cubeIndices, "earth");
    int moonMesh = registry.registerMesh(cubeVerts, cubeIndices, "cube");

    int platformMesh = registry.registerMesh(cubeVerts, cubeIndices, "cube");
    int lampMesh = registry.registerMesh(cubeVerts, cubeIndices, "cube");
    int grassMesh = registry.registerMesh(squareVerts, squareIndices, "square");

    for (int i = 0; i < COLORED_BLOCK_SIZE; i++) {
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
            .diffuseTexture = coloredBlockTextures[14],
            .specularTexture = (int)fallbackSpecular,
            .alpha = 0.5f
            });

    int simpleLamp = EntityBuilder::create(registry, "simpleLamp", glm::vec3(15.0f, 5.0f, 15.0f), glm::vec3(1.0f), root)
        .block(cubeMesh, litShader, {
            .isTransparent = false,
            .diffuseTexture = (int)texLamp,
            .specularTexture = (int)fallbackSpecular,
            .emissive = 0.75f,
            })
            .pointLight({ .color = { 1.0f, 0.95f, 0.85f }, .intensity = 0.5f });

    int coloredBlockIds[COLORED_BLOCK_SIZE];
    for (int i = 0; i < COLORED_BLOCK_SIZE; i++) {
        coloredBlockIds[i] = EntityBuilder::create(registry, colorNames[i] + " Block", glm::vec3(0.0f), glm::vec3(1.0f), root)
            .block(cubeMesh, litShader, {
                .isTransparent = false,
                .diffuseTexture = coloredBlockTextures[i],
                .specularTexture = (int)fallbackSpecular,
                });

    }

    int world = WorldBuilder::create(registry, "world", 64, 64, 64, worldShader, outlineShader)
        .platform(coloredBlockIds[13] /*green block*/, 64, 64)
        .outlineColor(engineSettings.outlineColor);

    player.setWorld(registry.getWorld(world));
    World& worldptr = *registry.getWorld(world);
    worldptr.importWorldFromPath(registry, WORLD_DIR + ui.worldFile());
    std::cout << "main: player @ " << &player << " world set to " << &world << std::endl;

    ui.buildBlockPalette(registry);
    if (!ui.palette().empty()) {
        player.changeSelectedBlock(ui.palette().front().entity);
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

        UIContext uiCtx{ registry, player, world, dirLight, lamp, WORLD_DIR };
        ui.render(uiCtx);

        glfwSwapBuffers(window);
    }

    ui.shutdown();
    glfwTerminate();

    return 0;
}

GLFWwindow* initOpenGL()
{
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        });

    std::cout << "Starting GLFW init..." << std::endl;

    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
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
    glEnable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return window;
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
        .pointLight({ .color = glm::vec3(1.0f), .intensity = 0.5f });

    int earth = EntityBuilder::create(registry, "earth", { 3.0f, 0.0f, 0.0f }, glm::vec3(0.5f), solarSystem)
        .renderable(earthMeshId, litShader, {
            .diffuseTexture = (int)texWorld, .specularTexture = (int)texWorld,
            .color = { 0.2f, 0.4f, 0.9f }, .shininess = 16.0f
            })
        .script(earthOrbitBehavior(0.25f, 0.25f, 23.5f));

    EntityBuilder::create(registry, "moon", { 1.5f, 0.0f, 0.0f }, glm::vec3(0.5f), earth)
        .renderable(moonMeshId, litShader, {
            .diffuseTexture = (int)texMoon, .specularTexture = (int)texMoon,
            .color = { 0.2f, 0.4f, 0.9f }, .shininess = 0.0f
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
        .pointLight({ .color = { 1.0f, 0.95f, 0.85f }, .intensity = 2.5f });

    outDirLight = EntityBuilder::create(registry, "dirLight", glm::vec3(0.0f), glm::vec3(1.0f), parentEntity)
        .dirLight({ .direction = glm::normalize(glm::vec3(0.5f, -1.5f, -0.8f)), .color = { 1.0f, 0.98f, 0.9f }, .intensity = 1.0f });
}

void setupVegetation(Registry& registry, int litShader, int parentEntity, int grassMeshId, unsigned int grassTex, unsigned int fallbackSpecular)
{
    const glm::vec3 positions[] = {
        { 14.0f, 3.0f, 16.0f },{ 15.0f, 3.0f, 16.5f },{ 8.0f, 3.0f, 16.0f },{ 8.0f, 3.0f, 16.5f }
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

    AssimpImporter importer(registry, MODEL_DIR + "backpack/backpack.obj", litShader);
    importer.loadModel("backpack", { .position = { 0.0f, 5.0f, 0.0f } }, root);

    setupSolarSystem(registry, litShader, root, sunMeshId, earthMeshId, moonMeshId, texSun, texWorld, texMoon, fallbackSpecular);
    setupLights(registry, litShader, root, lampMeshId, fallbackDiffuse, fallbackSpecular, outDirLight, outLamp);
    setupVegetation(registry, litShader, root, grassMeshId, grassTex, fallbackSpecular);

    return root;
}