cmake_minimum_required(VERSION 3.15)
project(OpenGLApp VERSION 1.0.0 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: link the MSVC runtime statically so users don't need the
# Visual C++ Redistributable. Must be set before add_executable and before
# the dependencies are fetched. Uncomment if you want it.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include(FetchContent)

# Build fetched libraries into compilation
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# ---------------------------------------------------------------------------
# GLFW
# ---------------------------------------------------------------------------
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

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
    GIT_PROGRESS   TRUE
)

# ---------------------------------------------------------------------------
# GLM
# ---------------------------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
    GIT_PROGRESS   TRUE
)

# ---------------------------------------------------------------------------
# Assimp
# ---------------------------------------------------------------------------
set(ASSIMP_BUILD_TESTS                     OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS              OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB                      ON  CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL                         OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS              OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT                       ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT  OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER              ON  CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER             ON  CACHE BOOL "" FORCE)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.4.1
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(glfw glm assimp)

find_package(OpenGL REQUIRED)

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------
# src
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# external
file(GLOB EXTERNAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/stb/*.cpp"
)
list(APPEND EXTERNAL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/glad.c"
)

add_executable(${PROJECT_NAME} ${GAME_SOURCES} ${EXTERNAL_SOURCES})

# headers
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# external headers (SYSTEM suppresses their warnings)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
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

# Strip debug symbols from Release builds on GCC/Clang (smaller exe)
if(NOT MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-s>)
endif()

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

# Install
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
install(DIRECTORY assets DESTINATION .)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/Release/${CMAKE_SYSTEM_NAME}_v${PROJECT_VERSION}" CACHE PATH "Install path" FORCE)
endif()