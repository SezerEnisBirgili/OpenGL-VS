#pragma once
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "lightingSets.h"
#include "camera.h"

// Binds textures and pushes all uniforms that stay constant after init.
class ShaderUniform 
{
    Shader& m_shader;

    const unsigned int       m_texture1;
    const unsigned int       m_texture2;

    const glm::mat4&          m_model;
    const Camera&            m_camera;
    const glm::mat4&          m_projection;

    const Material           m_material;  
    const Light              m_light;     

public:

    ShaderUniform(
        Shader& shader,
        const unsigned int texture1,
        const unsigned int texture2,
        const glm::mat4& model,
        const Camera& camera,
        const glm::mat4& projection,
        const Material   material,
        const Light      light)
        : m_shader(shader)
        , m_texture1(texture1)
        , m_texture2(texture2)
        , m_model(model)
        , m_camera(camera)
        , m_projection(projection)
        , m_material(material)
        , m_light(light)
    {
    }

    inline void initShaderUniforms()
    {
        m_shader.use();
        m_shader.setMat4("model", m_model);
        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);
        m_shader.setVec3("viewPos", m_camera.Position);

        // Material
        m_shader.setVec3("material.ambient", m_material.ambient);
        m_shader.setFloat("material.shininess", m_material.shininess);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture1);
        m_shader.setInt("material.diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture2);
        m_shader.setInt("material.specular", 1);

        // Light
        m_shader.setVec3("light.ambient", m_light.ambient);
        m_shader.setVec3("light.diffuse", m_light.diffuse);
        m_shader.setVec3("light.specular", m_light.specular);
        m_shader.setVec3("light.direction", m_light.direction);
        m_shader.setFloat("light.constant", m_light.constant);
        m_shader.setFloat("light.linear", m_light.linear);
        m_shader.setFloat("light.quadratic", m_light.quadratic);
        m_shader.setFloat("light.cutOff", m_light.cutOff);
        m_shader.setFloat("light.outerCutOff", m_light.outerCutOff);

        // Flash-light style: position/direction come from the camera
        m_shader.setVec3("light.position", m_camera.Position);
        m_shader.setVec3("light.direction", m_camera.Front);
    }

    // Called every frame – updates only the uniforms that can change at runtime.
    inline void updateFrameUniforms()
    {
        m_shader.use();
        m_shader.setMat4("model", m_model);
        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);
        m_shader.setVec3("viewPos", m_camera.Position);
        m_shader.setVec3("light.position", m_camera.Position);
        m_shader.setVec3("light.direction", m_camera.Front);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture2);
    }
};
