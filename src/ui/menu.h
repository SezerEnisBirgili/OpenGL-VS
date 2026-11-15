#pragma once

#include <string>
#include <vector>

#include "entities/GameObject.h"
#include "entities/player.h"

struct GLFWwindow;

struct BlockPaletteEntry {
    int entity;
    std::string name;
    unsigned int diffuseTexture;
};

struct UIContext {
    Registry& reg;
    Player& player;
    int worldEntity;
    int dirLight;
    int lamp;
    const std::string& worldDir;
};

class ImguiMenu {
public:
    void init(GLFWwindow* window);
    void shutdown();

    void buildBlockPalette(Registry& reg);
    const std::vector<BlockPaletteEntry>& palette() const { return blockPalette; }
    const char* worldFile() const { return worldFileBuf; }

    void render(UIContext& ctx);

private:
    void drawCrosshair();
    void drawCameraSection(UIContext& ctx);
    void drawWorldIO(UIContext& ctx);
    void drawLighting(UIContext& ctx);
    void drawWorldSettings(UIContext& ctx);
    void drawBlockPalette(UIContext& ctx);
    void drawCullingDebug();

    std::vector<BlockPaletteEntry> blockPalette;
    char worldFileBuf[128] = "world.txt";
};