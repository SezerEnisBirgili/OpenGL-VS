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
#include "horrorWorld.h"
#include "lightingSets.h"
#include "globals.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// -------------------------------------------------------------------------
// Callbacks
// -------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// -------------------------------------------------------------------------
// Input
// -------------------------------------------------------------------------
void processInput(GLFWwindow* window, Shader& ourShader);
void loadTexture(const char* texFileName, unsigned int& texture1);


// =========================================================================
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
    // Shaders
    // ------------------------------------------------------------------
    std::cout << "Compiling shaders..." << std::endl;

    Shader ourShader(vertexShader[0], fragmentShader[0]);
    Shader lightCubeShader(vertexShader[1], fragmentShader[1]);

    for(int i = 0; i < std::size(vertexShader); i++)
        std::cout << "Loading vertex shader: " << vertexShader[i] << std::endl;
    for (int i = 0; i < std::size(fragmentShader); i++)
        std::cout << "Loading vertex shader: " << fragmentShader[i] << std::endl;

    // ------------------------------------------------------------------
    // Vertex data
    // ------------------------------------------------------------------
    float vertices[] = {
        //vertex              //normal             //texCoord
        // Face 0 (back, -Z)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        // Face 1 (front, +Z)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        // Face 2 (left, -X)
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        // Face 3 (right, +X)
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         // Face 4 (bottom, -Y)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        // Face 5 (top, +Y)
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glEnable(GL_DEPTH_TEST);

    // ------------------------------------------------------------------
    // Texture
    // ------------------------------------------------------------------
    std::cout << "Loading texture..." << std::endl;

    unsigned int texture1, texture2;
    loadTexture("container2.png", texture1);
    loadTexture("container2.png", texture2);
    // ------------------------------------------------------------------
    // Shader uniforms
    // ------------------------------------------------------------------
    std::cout << "Setting uniforms..." << std::endl;

    ourShader.use();
    ourShader.setMat4("model", model);
    ourShader.setMat4("view", view);
    ourShader.setMat4("projection", projection);

    ourShader.setVec3("viewPos", camera.Position);

    ourShader.setVec3("material.ambient", material.ambient);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    ourShader.setInt("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    ourShader.setInt("material.specular", 1);
    glActiveTexture(GL_TEXTURE2);

    ourShader.setFloat("material.shininess", material.shininess);

    ourShader.setVec3("light.ambient", light.ambient);
    ourShader.setVec3("light.diffuse", light.diffuse);
    ourShader.setVec3("light.specular", light.specular);
    ourShader.setVec3("light.direction", light.direction);

    ourShader.setFloat("light.constant", light.constant);
    ourShader.setFloat("light.linear", light.linear);
    ourShader.setFloat("light.quadratic", light.quadratic);

    ourShader.setVec3("light.position", camera.Position);
    ourShader.setVec3("light.direction", camera.Front);

    ourShader.setFloat("light.cutOff", light.cutOff);
    ourShader.setFloat("light.outerCutOff", light.outerCutOff);

    // ------------------------------------------------------------------
    // IMGUI
    // ------------------------------------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::vector<std::vector<std::string>> floors = generateWorld(wallFilePath);
    glm::vec3 startPos = glm::vec3(0.0f);
    float rotation = 0.0f;

    // ------------------------------------------------------------------
    // Render loop
    // ------------------------------------------------------------------
    std::cout << "Entering render loop..." << std::endl;
    while (!glfwWindowShouldClose(window))
    {
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, ourShader);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
       
        // Update per-frame uniforms
        ourShader.use();
        ourShader.setMat4("model", model);
        ourShader.setMat4("view", camera.GetViewMatrix());
        ourShader.setMat4("projection", projection);
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setVec3("light.position", camera.Position);
        ourShader.setVec3("light.direction", camera.Front);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(cubeVAO);

        renderWorld(ourShader, floors, startPos, rotation);

        ImGui::SetNextWindowSize(ImVec2(250, 200), ImGuiCond_Once);
        ImGui::Begin("Controls");              

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
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
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

    if (!TOOGLE_MENU)
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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
    if (!TOOGLE_MENU) 
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}

void loadTexture(const char* texFileName, unsigned int& texture1)
{
    std::cout << "loadTexture: " << texFileName << std::endl;

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texFileName, &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1) format = GL_RED;
        else if (nrChannels == 3) format = GL_RGB;
        else if (nrChannels == 4) format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    }
    else
    {
        std::cout << "Failed to load texture: " << stbi_failure_reason() << std::endl;
    }
    stbi_image_free(data);
}