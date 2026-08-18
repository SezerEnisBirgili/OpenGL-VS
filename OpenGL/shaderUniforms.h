#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include "camera.h"


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
    const Camera& m_camera;
    const glm::mat4& m_projection;

public:
    LightCubeShaderUniform(Shader& shader, const Camera& camera, const glm::mat4& projection)
        : m_shader(shader)
        , m_camera(camera)
        , m_projection(projection)
    {
    }

    inline void initShaderUniforms() override
    {
        m_shader.use();
        m_shader.setMat4("projection", m_projection);
    }

    inline void updateFrameUniforms() override
    {
        m_shader.use();
        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);
    }
};

struct PointLightParams {
    glm::vec3 ambient = glm::vec3(0.05f, 0.05f, 0.05f);
    glm::vec3 diffuse = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
    float constant = 1.0f;
    float linear = 0.09f;
    float quadratic = 0.032f;
};


class OurShaderUniform : public ShaderUniform
{
    Shader& m_shader;
    const Camera& m_camera;
    const glm::mat4& m_projection;

    const glm::vec3* m_pointLightPositions; // array of 4
    PointLightParams m_pointLightParams;    

public:
    OurShaderUniform(
        Shader& shader,
        const Camera& camera,
        const glm::mat4& projection,
        const glm::vec3* pointLightPositions,
        PointLightParams pointLightParams = PointLightParams())

        : m_shader(shader)
        , m_camera(camera)
        , m_projection(projection)
        , m_pointLightPositions(pointLightPositions)
        , m_pointLightParams(pointLightParams)
    {
    }

    inline void initShaderUniforms() override
    {
        m_shader.use();
        m_shader.setMat4("projection", m_projection);

        // Directional light
        m_shader.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        m_shader.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        m_shader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        m_shader.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

        // Point lights
        setPointLights();

        // Spotlight
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

        m_shader.setMat4("view", m_camera.GetViewMatrix());
        m_shader.setMat4("projection", m_projection);
        m_shader.setVec3("viewPos", m_camera.Position);

        m_shader.setVec3("spotLight.position", m_camera.Position);
        m_shader.setVec3("spotLight.direction", m_camera.Front);
    }

private:
    inline void setPointLights()
    {
        for (int i = 0; i < 4; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "].";
            m_shader.setVec3(base + "position", m_pointLightPositions[i]);
            m_shader.setVec3(base + "ambient", m_pointLightParams.ambient);
            m_shader.setVec3(base + "diffuse", m_pointLightParams.diffuse);
            m_shader.setVec3(base + "specular", m_pointLightParams.specular);
            m_shader.setFloat(base + "constant", m_pointLightParams.constant);
            m_shader.setFloat(base + "linear", m_pointLightParams.linear);
            m_shader.setFloat(base + "quadratic", m_pointLightParams.quadratic);
        }
    }

    inline void setPointLightPositions()
    {
        for (int i = 0; i < 4; i++)
        {
            std::string base = "pointLights[" + std::to_string(i) + "].position";
            m_shader.setVec3(base, m_pointLightPositions[i]);
        }
    }
};