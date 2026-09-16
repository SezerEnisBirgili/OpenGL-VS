# OpenGL Engine

A small C++/OpenGL engine I'm using to build out an ECS and basic lighting. It has a small voxel world you can place and break blocks in. The engine is capable of importing arbitrary models through Assimp (there's a backpack model as a demo) and has a toy solar system with orbiting/spinning entities driven by simple script components.

## Features

- ECS-ish `Registry` transforms, materials, meshes, lights, hierarchy, scripts, all stored per-entity in hashmaps.
- `EntityBuilder`/`WorldBuilder` for chaining entity setup (e. g. `.renderable()` -> `.script()` -> `.pointLight()`).
- Parent/child transform hierarchy with world matrices recomputed each frame.
- A voxel grid featuring block breaking and placing through raycasting via DDA. It can export/import the grid to a text file.
- Model loading through Assimp (`AssimpImporter`) into the registry.
- There are essentially three shaders: 
  - world, for instanced rendering of the block system
  - outline, specifically for outlining the selected block in the block system
  - lit (the main one), which everything else treated as a game entity uses. 

  The main and world shaders both support directional, point, and specular lighting.
- Texture loading with dedup-by-path and fallback textures if diffuse/specular are missing, so it doesn't just crash on a bad material.
- An ImGui panel for camera info, light tweaking, outline color, world export/import, and a block palette you can click to select what you're placing.
- Materials can be alpha-blended and are sorted back-to-front each frame (e.g. Red Glass Block) or alpha-masked (e.g. grass).
- `.renderable()` entities are drawn individually as normal scene objects while `.block()` entities are registered into the block palette and drawn instanced through the world grid instead.


## Controls

- `WASD` — move
- `Space` / `Left Shift` — up / down
- Mouse — look
- Scroll — zoom
- Left click — break block
- Right click — place block
- `Left Ctrl` — toggle menu / free the cursor
- `Esc` — quit

## Setup

You need CMake 3.15+ and a C++20 compiler.

**Windows:** 
- Download Visual Studio from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads/) with the "Desktop development with C++" option that gets you both MSVC and CMake. 
- If cmake isn't found, download it from [cmake.org/download](https://cmake.org/download/) and make sure to add it to PATH during install. Visual Studio itself can be downloaded from 

**Linux:**

you will need the usual build tools plus X11/Wayland dev headers:

```bash
# Debian/Ubuntu
sudo apt install build-essential cmake libx11-dev libwayland-dev libxkbcommon-dev libgl1-mesa-dev

# Fedora
sudo dnf install @development-tools cmake libX11-devel wayland-devel libxkbcommon-devel mesa-libGL-devel
```

Then:

```bash
git clone https://github.com/SezerEnisBirgili/OpenGL-VS.git
cd OpenGL

cmake -S . -B build
cmake --build build --config Release
```

First build takes a few minutes because Assimp is compiling from source. After that it's incremental.

- `CMP0175` policy warnings during configure are coming from Assimp's own CMakeLists, not this project, ignore them.

To run:

```bash
# Linux
./OpenGLApp

# Windows
.\OpenGLApp.exe
```

if you are using Visual Studio and OpenGLApp.exe is not where it is supposed to be, by default Visual Studio may have dropped the exe in `.\Release\OpenGLApp.exe` or `.\Debug\OpenGLApp.exe` instead of the root.

## Rebuilding

You only need to rerun `cmake -S . -B build` if you touch `CMakeLists.txt` or add/remove source files. Otherwise just:

```bash
cmake --build build --config Release
```

## Layout

```
CMakeLists.txt                     build config
main.cpp                           entry point, scene setup, ImGui panel, main loop
engine.h                           wraps registry, player and systems
GameObject.h/.cpp                  registry, components, builders
world.h/.cpp                       voxel grid, raycasting, save/load grid
player.h/.cpp                      look/place/break block logic
camera.h                           free-fly camera
input.h                            GLFW callbacks + polling
RenderSystem.h/.cpp                draw passes, instancing, outline pass
IRenderable.h                      interface world/renderables implement to collect draw items
shader.h / mesh.h                  GL wrappers
AssimpImporter.h/.cpp              model loading into the registry
scripts.h                          scripted behaviors
utils.h                            utility functions (e.g. DDA raycast, AABB ray helpers)
settings.h                         engine/global light settings structs
vertex.h / vertexData.h            vertex layout + built-in cube/square geometry
enum.h                             shared enums
lit.vert/frag, outline.vert/frag   shaders
```

## Limitations

- Light counts are capped in the shaders: `MAX_POINT_LIGHTS = 16`, `MAX_DIR_LIGHTS = 4`. Lights beyond that silently stop showing up.
