cmake_minimum_required(VERSION 3.15)
project(OpenGLApp LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Build all fetched libraries statically (no DLLs to copy next to the exe)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---------------------------------------------------------------------------
# GLFW
# ---------------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XORG QUIET x11 xrandr xinerama xcursor xi)

    if(NOT XORG_FOUND)
        message(FATAL_ERROR "X11 development packages not found.")
    endif()

    set(GLFW_BUILD_X11     ON  CACHE BOOL "" FORCE)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)

# ---------------------------------------------------------------------------
# GLM
# ---------------------------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# ---------------------------------------------------------------------------
# Assimp
# ---------------------------------------------------------------------------
set(ASSIMP_BUILD_TESTS          OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS   OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB           ON  CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL              OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS   OFF CACHE BOOL "" FORCE)  # avoids failures on newer compilers

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.1
)

FetchContent_MakeAvailable(glfw glm assimp)

find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
# Your own code (recursive)
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Third-party code vendored in the repo (explicit, non-recursive)
file(GLOB EXTERNAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/*.cpp"
)
list(APPEND EXTERNAL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c"
)

add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${EXTERNAL_SOURCES})

# Your headers: #include "renderer/shader.h"
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Vendored third-party headers (SYSTEM suppresses their warnings)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui         # <imgui.h>, <imgui_impl_glfw.h>
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb           # "stb_image.h"
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include  # <glad/glad.h>, <KHR/khrplatform.h>
)

# Root folder for runtime assets (used by src/core/paths.h and main.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glm::glm
    assimp
    OpenGL::GL
    ${CMAKE_DL_LIBS}
)

# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------
# Put the exe in <build>/bin instead of the source tree
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Visual Studio: make this the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Visual Studio: show source files in folders matching the disk layout
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${GAME_SOURCES} ${EXTERNAL_SOURCES})