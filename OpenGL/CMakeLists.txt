cmake_minimum_required(VERSION 3.15)
project(OpenGLApp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)   # uses system libglfw.so from apt
find_package(OpenGL REQUIRED)

# Grab every .cpp and .c sitting flat in this folder
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/*.cpp" "${CMAKE_SOURCE_DIR}/*.c")

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}                          # shader.h, camera.h, etc.
    ${CMAKE_SOURCE_DIR}/Dependencies/include      # glad, GLFW headers, glm, KHR
)

target_link_libraries(${PROJECT_NAME} PRIVATE glfw OpenGL::GL ${CMAKE_DL_LIBS})

# Keep binary next to textures/shaders so relative paths in code still work
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)