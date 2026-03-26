#pragma once
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "globals.h"
#include "lightingSets.h"

// Binds textures and pushes all uniforms that stay constant after init.
inline void initShaderUniforms(Shader& ourShader,
    unsigned int texture1,
    unsigned int texture2)
{
    ourShader.use();
    ourShader.setMat4("model", model);
    ourShader.setMat4("view", camera.GetViewMatrix());
    ourShader.setMat4("projection", projection);
    ourShader.setVec3("viewPos", camera.Position);

    // Material
    ourShader.setVec3("material.ambient", material.ambient);
    ourShader.setFloat("material.shininess", material.shininess);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    ourShader.setInt("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    ourShader.setInt("material.specular", 1);

    // Light
    ourShader.setVec3("light.ambient", light.ambient);
    ourShader.setVec3("light.diffuse", light.diffuse);
    ourShader.setVec3("light.specular", light.specular);
    ourShader.setVec3("light.direction", light.direction);
    ourShader.setFloat("light.constant", light.constant);
    ourShader.setFloat("light.linear", light.linear);
    ourShader.setFloat("light.quadratic", light.quadratic);
    ourShader.setFloat("light.cutOff", light.cutOff);
    ourShader.setFloat("light.outerCutOff", light.outerCutOff);

    // Flash-light style: position/direction come from the camera
    ourShader.setVec3("light.position", camera.Position);
    ourShader.setVec3("light.direction", camera.Front);
}

// Called every frame – updates only the uniforms that can change at runtime.
inline void updateFrameUniforms(Shader& ourShader,
    unsigned int texture1,
    unsigned int texture2)
{
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
}