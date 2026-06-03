# OpenGL 3D Renderer

A real-time 3D renderer built from scratch in **C++ with OpenGL 3.3 Core Profile**. Developed as a learning project following the [learnopengl.com](https://learnopengl.com) curriculum, with several independent systems designed and implemented on top of the tutorial foundation.

![screenshot](screenshot.png)

---

## Features

### Lighting
- **Phong shading model** — ambient, diffuse, and specular components
- **Directional light** — simulates a global light source like the sun
- **Point lights (x4)** — positional lights with quadratic attenuation, rendered as small visible cubes in the scene
- **Spotlight / flashlight** — camera-attached spotlight with configurable inner and outer cutoff angles, toggled at runtime

### Camera System
Two distinct camera modes switchable at runtime via the ImGui overlay:

- **FREE mode** — standard FPS-style free-look camera. Mouse controls yaw and pitch, WASD moves in the direction the camera faces
- **TANK mode** — grid-based movement with smooth interpolation. W/S moves one unit forward/backward, A/D rotates exactly 90 degrees. Position and rotation animate smoothly using `deltaTime` rather than snapping instantly

The view matrix is computed using a **custom `lookAt` implementation** built from first principles — constructing the rotation matrix from the camera's right, up, and forward vectors and combining it with a translation matrix — rather than delegating to `glm::lookAt`.

### Rendering
- VAO/VBO setup with interleaved vertex data (position, normal, texture coordinates)
- Separate VAOs for scene objects and light cube geometry sharing a single VBO
- Texture loading with automatic format detection (GL_RED / GL_RGB / GL_RGBA) based on channel count
- Specular maps for per-pixel shininess control
- Depth testing enabled

### Shader Management
A `ShaderUniform` base class defines a two-phase uniform interface:
- `initShaderUniforms()` — uploads constants set once at startup (material properties, texture units, light attenuation parameters)
- `updateFrameUniforms()` — uploads per-frame data (view matrix, camera position, spotlight direction)

`OurShaderUniform` and `LightCubeShaderUniform` inherit from this base and manage their respective shaders independently.

### Procedural World Generation
A text file–based world system (`horrorWorld.cpp`) parses ASCII map files into multi-floor 3D geometry:
- `#` characters in the map file become rendered cubes at the corresponding XZ position
- Floors are separated by a `-` delimiter, stacked on the Y axis automatically
- Supports optional rotation of the entire generated structure

This allows level layouts to be edited in a plain text file without recompiling.

### Debug Overlay (ImGui)
A live control panel rendered via **Dear ImGui** exposes the following at runtime:
- Mouse mode toggle (FREE / TANK)
- Camera position and direction readout
- Ambient and attenuation factor sliders
- Light color picker
- Background color picker
- Flashlight on/off toggle

---

## Branch Overview

Each branch represents an independent feature implementation or experiment:

| Branch | Description |
|---|---|
| `master` | Stable build with full lighting, dual camera, ImGui overlay, and world generation |
| `multipleLights` | Implementation of multiple simultaneous light sources — directional, point (x4), and spotlight combined in a single fragment shader |
| `directionalLight` | Isolated directional light implementation, Phong shading with a single global light direction |
| `flashlight` | Spotlight attached to the camera with smooth edge falloff using inner/outer cutoff angles |
| `circularTextures` | Texture mapping experiments, circular/masked texture effects |
| `horror` | World generation system — ASCII map file parsing, multi-floor geometry, procedural cube placement |

---

## Built With

| Library | Purpose |
|---|---|
| OpenGL 3.3 Core | Graphics API |
| GLFW | Window creation and input handling |
| GLAD | OpenGL function pointer loading |
| GLM | Math library (vectors, matrices, transformations) |
| Dear ImGui | Immediate-mode debug UI overlay |
| stb_image | Texture loading |

---

## Project Structure

```
├── main.cpp              — Application entry point, render loop
├── shader.h/.cpp         — Shader program compilation and uniform setters
├── camera.h              — Camera class: FPS and tank modes, custom lookAt
├── input.h               — AppState: GLFW callbacks and per-frame input handling
├── horrorWorld.h/.cpp    — Procedural world generation from ASCII map files
├── bufferSetup.h         — VAO/VBO setup and vertex attribute configuration
├── textureLoader.h       — Texture loading with format auto-detection
├── shaderUniforms.h      — ShaderUniform class hierarchy for init/frame uniform management
├── lightingSets.h        — Material and Light structs with default values
├── vertexData.h          — Cube vertex data (position, normal, texcoord)
└── enum.h                — MouseState enum
```

---

## Building

This project was developed with Visual Studio 2022 on Windows x64.

**Dependencies required:**
- GLFW 3.x
- GLAD (OpenGL 3.3 Core, generated from [glad.dav1d.de](https://glad.dav1d.de))
- GLM
- Dear ImGui (with GLFW + OpenGL3 backends)
- stb_image

Link against: `opengl32.lib`, `glfw3.lib`

---

## What I Learned

- How the OpenGL pipeline works end to end: vertex data → VAO/VBO → vertex shader → rasterization → fragment shader → framebuffer
- How Phong lighting is computed per-fragment in GLSL, including attenuation and cutoff angles for spotlights
- How view and projection matrices work mathematically, including implementing `lookAt` from scratch
- How to structure a growing C++ OpenGL project into focused, reusable components
- How to integrate Dear ImGui into an OpenGL/GLFW application for real-time debugging
