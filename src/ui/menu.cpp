#include "ui/menu.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/settings.h"
#include "core/input.h"
#include "world/world.h"

#include <cstdint>
#include <iostream>

const char* const mouseStateArr[] = { "FREE" };
MouseState mouseState = MouseState::FREE;

void ImguiMenu::init(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    io.DisplayFramebufferScale = ImVec2((float)fbWidth / engineSettings.screenWidth, (float)fbHeight / engineSettings.screenHeight);
    glViewport(0, 0, fbWidth, fbHeight);
}

void ImguiMenu::shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImguiMenu::buildBlockPalette(Registry& reg)
{
    blockPalette.clear();

    for (int e : reg.blockPalette) 
    {
        auto matIt = reg.materials.find(e);
        if (matIt == reg.materials.end()) continue;
        if (matIt->second.isAir) continue;

        std::string name = reg.names.count(e) ? reg.names.at(e) : ("entity_" + std::to_string(e));
        blockPalette.push_back({ e, name, (unsigned int)matIt->second.diffuseTexture });
    }

    std::cout << "[ImGuiLayer] found " << blockPalette.size() << " block entities" << std::endl;
}

// ---------------------------------------------------------------------------
// Frame
// ---------------------------------------------------------------------------
void ImguiMenu::render(UIContext& ctx)
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawCrosshair();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(350.0f, (float)engineSettings.screenHeight - 20.0f), ImGuiCond_Once);
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    drawCameraSection(ctx);
    drawWorldIO(ctx);
    drawLighting(ctx);
    drawWorldSettings(ctx);
    drawBlockPalette(ctx);
    drawCullingDebug();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// ---------------------------------------------------------------------------
// Sections
// ---------------------------------------------------------------------------
void ImguiMenu::drawCrosshair()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 c(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    const float size = 10.0f, thickness = 2.0f;
    const ImU32 color = IM_COL32(255, 255, 255, 220);

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    dl->AddLine(ImVec2(c.x - size, c.y), ImVec2(c.x + size, c.y), color, thickness);
    dl->AddLine(ImVec2(c.x, c.y - size), ImVec2(c.x, c.y + size), color, thickness);
}

void ImguiMenu::drawCameraSection(UIContext& ctx)
{
    Camera& camera = ctx.player.getCamera();

    int selected = (int)mouseState;
    if (ImGui::Combo("Mouse Mode", &selected, mouseStateArr, IM_ARRAYSIZE(mouseStateArr)))
        mouseState = (MouseState)selected;

    ImGui::InputFloat3("Camera Position", glm::value_ptr(camera.Position));
    ImGui::InputFloat3("Camera Direction", glm::value_ptr(camera.Front));
}

void ImguiMenu::drawWorldIO(UIContext& ctx)
{
    World& world = *ctx.reg.getWorld(ctx.worldEntity);
    std::string fullPath = ctx.worldDir + worldFileBuf;

    if (ImGui::Button("Export World")) {
        if (world.exportWorldToPath(fullPath))
            std::cout << "World exported to " << fullPath << std::endl;
        else
            std::cout << "World export failed!" << std::endl;
    }

    ImGui::SameLine();

    if (ImGui::Button("Import World")) {
        world.importWorldFromPath(ctx.reg, fullPath);
        ctx.player.setWorld(&world);
        std::cout << "World imported from " << fullPath << std::endl;
    }
}

void ImguiMenu::drawLighting(UIContext& ctx)
{
    if (!ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) return;

    Registry& reg = ctx.reg;

    ImGui::SliderFloat("Ambient Strength", &lightSettings.ambientStrength, 0.0f, 1.0f);
    ImGui::ColorEdit3("Ambient Color", glm::value_ptr(lightSettings.ambientColor));
    ImGui::Checkbox("Directional Light Enabled", &lightSettings.dirLightEnabled);
    ImGui::Checkbox("Point Lights Enabled", &lightSettings.pointLightsEnabled);

    ImGui::Separator();
    ImGui::Text("Sun");
    if (reg.dirLights.count(ctx.dirLight)) {
        auto& light = reg.dirLights[ctx.dirLight];
        if (ImGui::SliderFloat3("Sun Direction", glm::value_ptr(light.direction), -1.0f, 1.0f)) {
            if (glm::length(light.direction) > 0.0001f)
                light.direction = glm::normalize(light.direction);
        }
        ImGui::ColorEdit3("Sun Color", glm::value_ptr(light.color));
        ImGui::SliderFloat("Sun Intensity", &light.intensity, 0.0f, 5.0f);
    }

    ImGui::Separator();
    ImGui::Text("Lamp");
    if (reg.pointLights.count(ctx.lamp)) {
        auto& lamp = reg.pointLights[ctx.lamp];
        ImGui::ColorEdit3("Lamp Color", glm::value_ptr(lamp.color));
        ImGui::SliderFloat("Lamp Intensity", &lamp.intensity, 0.0f, 10.0f);
        ImGui::SliderFloat("Lamp Linear", &lamp.linear, 0.0f, 1.0f);
        ImGui::SliderFloat("Lamp Quadratic", &lamp.quadratic, 0.0f, 2.0f);
    }
    if (reg.materials.count(ctx.lamp)) {
        ImGui::SliderFloat("Lamp Emissive", &reg.materials[ctx.lamp].emissive, 0.0f, 1.0f);
    }
}

void ImguiMenu::drawWorldSettings(UIContext& ctx)
{
    if (!ImGui::CollapsingHeader("World", ImGuiTreeNodeFlags_DefaultOpen)) return;

    if (ImGui::ColorEdit3("Outline Color", glm::value_ptr(engineSettings.outlineColor)))
        ctx.reg.getWorld(ctx.worldEntity)->setOutlineColor(engineSettings.outlineColor);
}

void ImguiMenu::drawBlockPalette(UIContext& ctx)
{
    if (!ImGui::CollapsingHeader("Block Palette", ImGuiTreeNodeFlags_DefaultOpen)) return;

    int currentBlock = ctx.player.getBlockToPlace();

    for (const auto& entry : blockPalette) {
        ImGui::PushID(entry.entity);

        if (entry.diffuseTexture != 0) {
            ImGui::Image((ImTextureID)(intptr_t)entry.diffuseTexture, ImVec2(32, 32));
            ImGui::SameLine();
        }

        if (ImGui::Selectable(entry.name.c_str(), entry.entity == currentBlock))
            ctx.player.changeSelectedBlock(entry.entity);

        ImGui::PopID();
    }
}

void ImguiMenu::drawCullingDebug()
{
    if (!ImGui::CollapsingHeader("Face Culling Debug", ImGuiTreeNodeFlags_DefaultOpen)) return;

    ImGui::Checkbox("Cull Face", &debugSettings.cullFace);
    ImGui::Checkbox("Wireframe", &debugSettings.wireframe);
    ImGui::Checkbox("Show Front/Back Colors", &debugSettings.showFaceColors);

    ImGui::Separator();
    GLint cullMode, frontFace;
    glGetIntegerv(GL_CULL_FACE_MODE, &cullMode);
    glGetIntegerv(GL_FRONT_FACE, &frontFace);
    ImGui::Text("GL_CULL_FACE: %s", glIsEnabled(GL_CULL_FACE) ? "ON" : "OFF");
    ImGui::Text("Cull mode: %s", cullMode == GL_BACK ? "BACK" : cullMode == GL_FRONT ? "FRONT" : "FRONT_AND_BACK");
    ImGui::Text("Front face: %s", frontFace == GL_CCW ? "CCW" : "CW");
}