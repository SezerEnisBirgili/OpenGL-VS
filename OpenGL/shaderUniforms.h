#pragma once
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "lightingSets.h"
#include "camera.h"

// Binds textures and pushes all uniforms that stay constant after init.
class ShaderUniform
{
public:
    ShaderUniform() = default;
    virtual ~ShaderUniform() = default;
    virtual void initShaderUniforms() = 0;
    virtual void updateFrameUniforms() = 0;
};

class LightCubeShaderUniform : public ShaderUniform 
{
    Shader& m_shader;

    const glm::mat4& m_model;
    const Camera& m_camera;
    const glm::mat4& m_projection;

public:

    LightCubeShaderUniform(
        Shader& shader,
        const glm::mat4& model,
        const Camera& camera,
        const glm::mat4& projection)

        : m_shader(shader)
        , m_model(model)
        , m_camera(camera)
        , m_projection(projection)
    {
    }

    inline void initShaderUniforms() override
    {
        m_shader.use();
        
        m_shader.setMat4("model", m_model);
        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);

    }

    inline void updateFrameUniforms() override
    {
        m_shader.use();

        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);

    }
};

class OurShaderUniform : public ShaderUniform
{
    Shader& m_shader;

    const unsigned int       m_texture1;
    const unsigned int       m_texture2;

    const glm::mat4& m_model;
    const Camera& m_camera;
    const glm::mat4& m_projection;

    const Material           m_material;
    const Light              m_light;

    const glm::vec3* m_pointLightPositions;

public:

    OurShaderUniform(
        Shader& shader,
        const unsigned int texture1,
        const unsigned int texture2,
        const glm::mat4& model,
        const Camera& camera,
        const glm::mat4& projection,
        const Material     material,
        const Light        light,
        const glm::vec3* pointLightPositions)

        : m_shader(shader)
        , m_texture1(texture1)
        , m_texture2(texture2)
        , m_model(model)
        , m_camera(camera)
        , m_projection(projection)
        , m_material(material)
        , m_light(light)
        , m_pointLightPositions(pointLightPositions)
    {
    }

    inline void initShaderUniforms() override
    {
        m_shader.use();
        m_shader.setMat4("model", m_model);
        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);

        m_shader.setVec3("viewPos", m_camera.Position);

        // Material
        m_shader.setFloat("material.shininess", m_material.shininess);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture1);
        m_shader.setInt("material.diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture2);
        m_shader.setInt("material.specular", 1);

        // Light
                // directional light
        m_shader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        m_shader.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        m_shader.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // point light 1
        m_shader.setVec3("pointLights[0].position", m_pointLightPositions[0]);
        m_shader.setVec3("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        m_shader.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setFloat("pointLights[0].constant", 1.0f);
        m_shader.setFloat("pointLights[0].linear", 0.09f);
        m_shader.setFloat("pointLights[0].quadratic", 0.032f);
        // point light 2
        m_shader.setVec3("pointLights[1].position", m_pointLightPositions[1]);
        m_shader.setVec3("pointLights[1].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader.setVec3("pointLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        m_shader.setVec3("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setFloat("pointLights[1].constant", 1.0f);
        m_shader.setFloat("pointLights[1].linear", 0.09f);
        m_shader.setFloat("pointLights[1].quadratic", 0.032f);
        // point light 3
        m_shader.setVec3("pointLights[2].position", m_pointLightPositions[2]);
        m_shader.setVec3("pointLights[2].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader.setVec3("pointLights[2].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        m_shader.setVec3("pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setFloat("pointLights[2].constant", 1.0f);
        m_shader.setFloat("pointLights[2].linear", 0.09f);
        m_shader.setFloat("pointLights[2].quadratic", 0.032f);
        // point light 4
        m_shader.setVec3("pointLights[3].position", m_pointLightPositions[3]);
        m_shader.setVec3("pointLights[3].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader.setVec3("pointLights[3].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
        m_shader.setVec3("pointLights[3].specular", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setFloat("pointLights[3].constant", 1.0f);
        m_shader.setFloat("pointLights[3].linear", 0.09f);
        m_shader.setFloat("pointLights[3].quadratic", 0.032f);
        // spotLight
        m_shader.setVec3("spotLight.position", m_camera.Position);
        m_shader.setVec3("spotLight.direction", m_camera.Front);
        m_shader.setVec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        m_shader.setVec3("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        m_shader.setFloat("spotLight.constant", 1.0f);
        m_shader.setFloat("spotLight.linear", 0.09f);
        m_shader.setFloat("spotLight.quadratic", 0.032f);
        m_shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        m_shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

    }

    inline void updateFrameUniforms() override
    {
        m_shader.use();

        m_shader.setMat4("model", m_model);
        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);
        m_shader.setVec3("viewPos", m_camera.Position);

        m_shader.setVec3("spotLight.position", m_camera.Position);
        m_shader.setVec3("spotLight.direction", m_camera.Front);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_texture2);
    }
};