# OpenGL Engine

A small C++/OpenGL engine I'm using to build out an ECS and basic lighting. It has a small voxel world you can place and break blocks in. The engine is capable of importing arbitrary models through Assimp (there's a backpack model as a demo) and has a toy solar system with orbiting/spinning entities driven by simple script components.

The goal is to implement everything in LearnOpenGL within one engine.

## Features

- ECS-ish Registry transforms, materials, meshes, lights, hierarchy, scripts, all stored per-entity in hashmaps.
- EntityBuilder/WorldBuilder for chaining entity setup (e.g. `.renderable()` -> `.script()` -> `.pointLight()`).
- Parent/child transform hierarchy with world matrices recomputed each frame.
- A voxel grid featuring block breaking and placing through raycasting via DDA. It can export/import the grid to a text file.
- Model loading through Assimp (`AssimpImporter`) into the registry.
- There are essentially three shaders:
  - `world`, for instanced rendering of the block system
  - `outline`, specifically for outlining the selected block in the block system
  - `lit` (the main one), which everything else treated as a game entity uses.
- The main and world shaders both support directional, point, and specular lighting.
- Texture loading with dedup-by-path and fallback textures if diffuse/specular are missing, so it doesn't just crash on a bad material.
- An ImGui panel for camera info, light tweaking, outline color, world export/import, and a block palette you can click to select what you're placing.
- Materials can be alpha-blended and are sorted back-to-front each frame (e.g. Red Glass Block) or alpha-masked (e.g. grass).
- `.renderable()` entities are drawn individually as normal scene objects while `.block()` entities are registered into the block palette and drawn instanced through the world grid instead.

## Controls

- **WASD** — move
- **Space / Left Shift** — up / down
- **Mouse** — look
- **Scroll** — zoom
- **Left click** — break block
- **Right click** — place block
- **Left Ctrl** — toggle menu / free the cursor
- **Esc** — quit

## Setup

Requires Git, CMake 3.15+ and a C++20 compiler.

Dependencies: GLFW 3.4 and Assimp 5.4.1 are fetched by CMake on first configure. GLM, GLAD, Dear ImGui and stb_image are bundled.

### Windows

- Install [Visual Studio](https://visualstudio.microsoft.com/) with the "Desktop development with C++" workload (includes MSVC and CMake).
- Install [Git](https://git-scm.com/download/win).
- If `cmake` isn't found, install it from [cmake.org/download](https://cmake.org/download/) and add it to PATH.

### Linux

```bash
# Debian/Ubuntu
sudo apt install git build-essential cmake libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev

# Fedora
sudo dnf install git @development-tools cmake libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel
```

### Build

```bash
git clone https://github.com/SezerEnisBirgili/OpenGL-VS.git
cd OpenGL-VS/OpenGL

cmake -S . -B build
cmake --build build --config Release
```

### Run

From the `OpenGL/` folder:

```bash
# Linux
build/bin/OpenGLApp

# Windows
build\bin\OpenGLApp.exe
```

## Architecture

Entities are plain integer IDs. The `Registry` stores each component type (transform, material, mesh, light, script, hierarchy) in its own hashmap keyed by entity ID, and also owns the mesh, shader and world storage.

Each frame, `Engine::update` runs `transformSystem` (recomputes world matrices down the hierarchy) and then `scriptSystem` (runs every entity's script). `Engine::render` then calls `RenderSystem`, which collects lights and render items, sorts transparent items back-to-front, and draws opaque entities, instanced blocks and the selected-block outline.

Adding an entity:

```cpp
int lamp = EntityBuilder::create(registry, "Spinning Lamp",
                                 glm::vec3(15.0f, 5.0f, 15.0f), glm::vec3(1.0f), root)
    .renderable(cubeMesh, litShader, {
        .diffuseTexture  = (int)texLamp,
        .specularTexture = (int)fallbackSpecular,
        .emissive        = 0.75f })
    .script(spinBehavior(1.0f))
    .pointLight({ .color = {1.0f, 0.95f, 0.85f}, .intensity = 0.5f });
```

Scripts are plain callables with the signature `void(Registry&, int self, float time, float dt)`. See `scripts.h` for examples.

## Layout

```
CMakeLists.txt                     build config
main.cpp                           entry point, scene setup, ImGui panel, main loop
engine.h                           wraps registry, player and systems
GameObject.h / gameObject.cpp      registry, components, builders
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
lit.vert/frag, world.vert,
outline.vert/frag                  shaders
```

## Limitations

Light counts are capped in the shaders: `MAX_POINT_LIGHTS = 16`, `MAX_DIR_LIGHTS = 4`. Lights beyond that silently stop showing up.

## Credits

- Backpack model by Berk Gedik ([Sketchfab](https://sketchfab.com/3d-models/survival-guitar-backpack-low-poly-799f8c4511f84fab8c3f12887f7e6b36)), with material changes by Joey de Vries from [LearnOpenGL](https://learnopengl.com).
- `container2.png` and `container2_specular.png` are from LearnOpenGL.
- Libraries: GLFW, GLM, GLAD, Dear ImGui, stb_image, Assimp.