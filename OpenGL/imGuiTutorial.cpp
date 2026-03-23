#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "shader.h"
#include "camera.h"

#include <iostream>

// -------------------------------------------------------------------------
// Constants
// -------------------------------------------------------------------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char* vertexShader = "vTestShader.vert";
const char* fragmentShader = "fTestShader.frag";
// -------------------------------------------------------------------------
// Camera state
// -------------------------------------------------------------------------
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 5.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float r = 0.0f, g = 0.0f, b = 0.0f;
glm::vec3 rgb = glm::vec3(r, g, b);


float yaw = -90.0f;
float pitch = 0.0f;

Camera camera = Camera(cameraPos, cameraUp, yaw, pitch);

float lastX = 400, lastY = 300;
bool  firstMouse = true;

// -------------------------------------------------------------------------
// Timing
// -------------------------------------------------------------------------
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// -------------------------------------------------------------------------
// Rendering state
// -------------------------------------------------------------------------
float CUR_MIX_VALUE = 0.2f;
float fov = 45.0f;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = camera.GetViewMatrix();
glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

bool TOOGLE_MENU = false;

bool lShiftPressedLastFrame = false;

// -------------------------------------------------------------------------
// Callbacks
// -------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

void processInput(GLFWwindow* window, Shader& ourShader);


int main()
{

    // ------------------------------------------------------------------
    // GLFW: init and configure
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
    // GLFW: window creation
    // ------------------------------------------------------------------
    std::cout << "Creating window..." << std::endl;
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // ------------------------------------------------------------------
    // GLAD: load OpenGL function pointers
    // ------------------------------------------------------------------
    std::cout << "Loading GLAD..." << std::endl;
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // ------------------------------------------------------------------
    // Shader
    // ------------------------------------------------------------------
    std::cout << "Compiling shaders..." << std::endl;
    Shader ourShader(vertexShader, fragmentShader);
    std::cout << "Loading vertex shader: " << vertexShader << std::endl;
    std::cout << "Loading fragment shader: " << fragmentShader << std::endl;

    // ------------------------------------------------------------------
    // Vertex data
    // ------------------------------------------------------------------
    float vertices[] = {
        //vertex              //normal             //texCoord
        // Face 0 (back, -Z) — top-left cell [0.000, 0.333]
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.000f, 0.5f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.333f, 0.5f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.333f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.333f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.000f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.000f, 0.5f,

        // Face 1 (front, +Z) — top-middle cell [0.333, 0.666]
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.333f, 0.5f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.666f, 0.5f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.666f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.666f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.333f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.333f, 0.5f,

        // Face 2 (left, -X) — top-right cell [0.666, 1.000]
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.666f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.000f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.000f, 0.5f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.000f, 0.5f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.666f, 0.5f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.666f, 1.0f,

        // Face 3 (right, +X) — bottom-left cell [0.000, 0.333]
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.000f, 0.5f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.333f, 0.5f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.333f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.333f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.000f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.000f, 0.5f,

         // Face 4 (bottom, -Y) — bottom-middle cell [0.333, 0.666]
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.333f, 0.0f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.666f, 0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.666f, 0.5f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.666f, 0.5f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.333f, 0.5f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.333f, 0.0f,

         // Face 5 (top, +Y) — bottom-right cell [0.666, 1.000] (white/cloud face)
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.666f, 0.0f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.000f, 0.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.000f, 0.5f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.000f, 0.5f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.666f, 0.5f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.666f, 0.0f,
    };

    // ------------------------------------------------------------------
    // VAO / VBO
    // ------------------------------------------------------------------
    std::cout << "Uploading vertex data..." << std::endl;
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // ------------------------------------------------------------------
    glEnable(GL_DEPTH_TEST);

    std::cout << "Shader Uniforms..." << std::endl;

    ourShader.use();
    ourShader.setMat4("model", model);
    ourShader.setMat4("view", view);
    ourShader.setMat4("projection", projection);
    ourShader.setVec3("viewPos", camera.Position);
    ourShader.setVec3("rgb", rgb);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout << "Rendering..." << std::endl;

    // ----------------------------------------------------------------
    // Main loop
    // ----------------------------------------------------------------

    while (!glfwWindowShouldClose(window))
    {
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processInput(window, ourShader);

        // Rendering
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = (float)glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        rgb = glm::vec3(r, g, b);
        ourShader.use();
        ourShader.setMat4("view", camera.GetViewMatrix());
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("rgb", rgb);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // inside main loop, replace ShowDemoWindow with:
        ImGui::Begin("Controls");                              // creates a window titled "Controls"
        ImGui::SliderFloat("RED", &r, 0.0f, 1.0f);
        ImGui::SliderFloat("GREEN", &g, 0.0f, 1.0f);
        ImGui::SliderFloat("BLUE", &b, 0.0f, 1.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // ----------------------------------------------------------------
    // Cleanup
    // ----------------------------------------------------------------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}

// =========================================================================
// Callbacks
// =========================================================================
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (TOOGLE_MENU) return;

    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;
    lastX = (float)xpos;
    lastY = (float)ypos;

    if (!ImGui::GetIO().WantCaptureMouse)
        camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
}

// =========================================================================
// Input
// =========================================================================
void processInput(GLFWwindow* window, Shader& ourShader)
{
    // escape window

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // menu switch

    bool lShiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
    
    if (lShiftPressed && !lShiftPressedLastFrame)
    {
        TOOGLE_MENU = !TOOGLE_MENU;
        firstMouse = true;
        if (TOOGLE_MENU)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    lShiftPressedLastFrame = lShiftPressed;

    // Camera movement

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
}