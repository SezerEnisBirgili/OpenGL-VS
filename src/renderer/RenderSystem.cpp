#include "RenderSystem.h"
#include "IRenderable.h"
#include "core/settings.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL   // required for glm/gtx/* headers in GLM 1.0+
#include <glm/gtx/norm.hpp>       // glm::distance2

void RenderSystem::render(const DrawItem& item, int s) {

    Shader* shader = reg.shader(s ? s : item.material->shaderId);
    const Material& mat = *item.material;
    
    shader->use();

    bindFrameUniforms(*shader); // view, projection, viewPos, ambient, lights
    shader->setMat4("model", item.model);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mat.diffuse.textureID);
    shader->setInt("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mat.specular.textureID);
    shader->setInt("material.specular", 1);

    shader->setVec3("material.color", mat.color);
    shader->setFloat("material.shininess", mat.shininess);
    shader->setFloat("material.emissive", mat.emissive);
    shader->setFloat("material.alphaCutoff", mat.alphaCutoff);
    shader->setFloat("material.alpha", mat.alpha);
    shader->setBool("material.isMasked", mat.isMasked);

    // masked renderables (e.g. grass) are typically square meshes
    const bool noCull = debugSettings.cullFace && mat.isMasked;
    if (noCull) glDisable(GL_CULL_FACE);

    drawMesh(item.mesh);

    if (noCull) glEnable(GL_CULL_FACE);
}

/*
void RenderSystem::renderInstanced(const std::vector<InstancedRenderItem>& items, int shader) {
    for (const InstancedRenderItem& batch : items) {
        if (batch.positions.empty()) {
            std::cout << "[RenderSystem] skipping empty batch e=" << batch.e << std::endl;
            continue;
        }

        Shader* s = shader ? reg.getShaderWithId(shader) : reg.getShader(batch.e);
        Mesh* mesh = reg.getMesh(batch.e);
        const glm::mat4& world = reg.worldMatrices.at(batch.e).value;
        const MaterialComponent& mat = reg.materials.at(batch.e);

        s->use();

        s->setFloat("ambientStrength", lightSettings.ambientStrength);
        s->setVec3("ambientColor", lightSettings.ambientColor);
        s->setMat4("model", glm::mat4(1.0f)); // positions already resolved to world space
        s->setMat4("view", player.getCamera().GetViewMatrix());
        s->setMat4("projection", engineSettings.getProjectionMatrix());
        s->setVec3("viewPos", player.getCamera().Position);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
        s->setInt("material.diffuse", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mat.specularTexture);
        s->setInt("material.specular", 1);

        s->setVec3("material.color", mat.color);
        s->setFloat("material.shininess", mat.shininess);
        s->setFloat("material.emissive", mat.emissive);
        s->setFloat("material.alphaCutoff", mat.alphaCutoff);
        s->setFloat("material.alpha", mat.alpha);
        s->setBool("material.isMasked", mat.isMasked);

        s->setBool("debugFaces", debugSettings.showFaceColors);

        uploadLights(*s);
        uploadDirLights(reg, *s, lightSettings.dirLightEnabled);

        uploadInstancePositions(mesh, batch.positions);
        drawMeshInstanced(mesh, (int)batch.positions.size());
    }
}

void RenderSystem::uploadInstancePositions(Mesh* mesh, const std::vector<glm::vec3>& positions) {
    glBindVertexArray(mesh->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}
*/

void RenderSystem::drawMesh(const Mesh* mesh) {
    glBindVertexArray(mesh->VAO);
    glDisableVertexAttribArray(3);   // in case of leftover aInstancePos from drawMeshInstanced
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

/*
void RenderSystem::drawMeshInstanced(Mesh* mesh, int count) {
    glBindVertexArray(mesh->VAO);
    glDrawElementsInstanced(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0, count);
    glDisableVertexAttribArray(3);
    glBindVertexArray(0);
}
*/

void RenderSystem::collectGpuLights() {
    pointLightsGPU.clear();
    dirLightsGPU.clear();

    for (const LightItem& li : lights) {
        const Light& l = li.light;

        switch (l.type) {
        case LightType::Point: 
        {
            GpuPointLight g;
            g.position  = li.position;
            g.color     = l.color;
            g.intensity = l.intensity;
            g.constant  = l.constant;
            g.linear    = l.linear;
            g.quadratic = l.quadratic;
            pointLightsGPU.push_back(g);
            break;
        }
        case LightType::Directional: 
        {
            GpuDirLight g;
            g.direction = li.direction;
            g.color     = l.color;
            g.intensity = l.intensity;
            dirLightsGPU.push_back(g);
            break;
        }
        case LightType::Spot:
            break; //TODO: implement spotlights e.g. flashlight
        }
    }
}

void RenderSystem::uploadLights(Shader& shader) {
    shader.setBool("pointLightsEnabled", lightSettings.pointLightsEnabled);
    shader.setInt("numPointLights", (int)pointLights.size());

    for (size_t i = 0; i < pointLights.size(); i++) {
        const LightItem& li = pointLights[i];
        const Light& l = li.light;  
        std::string base = "pointLights[" + std::to_string(i) + "].";
        shader.setVec3(base + "position", li.position);
        shader.setVec3(base + "color", l.color);
        shader.setFloat(base + "intensity", l.intensity);
        shader.setFloat(base + "constant", l.constant);
        shader.setFloat(base + "linear", l.linear);
        shader.setFloat(base + "quadratic", l.quadratic);
    }
}

void RenderSystem::uploadDirLights(Shader& shader) {
    if (!lightSettings.dirLightEnabled) { shader.setInt("numDirLights", 0); return; }

    shader.setInt("numDirLights", (int)dirLights.size());

    for (size_t i = 0; i < dirLights.size(); i++) {
        const LightItem& li = dirLights[i];
        std::string base = "dirLights[" + std::to_string(i) + "].";
        shader.setVec3(base + "direction", li.direction);
        shader.setVec3(base + "color", li.light.color);
        shader.setFloat(base + "intensity", li.light.intensity);
    }
}

void RenderSystem::collectRenderItems() {
    opaque.clear();
    transparent.clear();
    pointLights.clear();
    dirLights.clear();

    for (const auto& [e, r] : scene.renderables) {
        if (!r.isVisible || r.modelId < 0) continue;

        const glm::mat4& world = worldMatrices.at(e);
        Model* model = reg.model(r.modelId);

        for (const ModelNode& node : model->nodes) {
            DrawItem item;
            item.mesh     = node.mesh;
            item.material = &r.material;
            item.model    = world * node.transform.toMat4();
            item.entity   = e;

            r.material.alpha < 1.0f ? transparent.push_back(item) : opaque.push_back(item);
        }
    }

    for (const auto& [e, l] : scene.lights) {
        const glm::mat4& w = worldMatrices.at(e);

        LightItem li;
        li.light     = l;
        li.position  = glm::vec3(w[3]);
        li.direction = glm::normalize(glm::vec3(w * glm::vec4(0, 0, -1, 0)));

        if (l.type == LightType::Point)            pointLights.push_back(li);
        else if (l.type == LightType::Directional) dirLights.push_back(li);
    }
}

void RenderSystem::applyDebugState() {
    if (debugSettings.cullFace) glEnable(GL_CULL_FACE);
    else                        glDisable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, debugSettings.wireframe ? GL_LINE : GL_FILL);
}

void RenderSystem::drawOpaquePass() 
{
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    for (const auto& item : opaque)
        render(item);

    /*
    for (int e : reg.renderableWorlds) {
        int worldShaderId = reg.worlds.at(e).shaderId;
        renderInstanced(instancedOpaque, worldShaderId);
    }
    */
}

void RenderSystem::drawTransparentPass() 
{
    const glm::vec3 camPos = player.getCamera().Position;

    std::sort(transparent.begin(), transparent.end(), 
    [&](const RenderItem& a, const RenderItem& b) { return glm::distance2(camPos, glm::vec3(a.model[3])) > glm::distance2(camPos, glm::vec3(b.model[3]));
    });

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto& item : transparent)
        render(item);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

const glm::mat4& RenderSystem::resolveWorld(int e) {
    auto it = worldMatrices.find(e);
    if (it != worldMatrices.end()) return it->second;
    
    glm::mat4 local = scene.transforms.at(e).toMatrix();

    int parent = scene.hierarchy.getParent(e);
    glm::mat4 world = (parent != NULL_ENTITY) ? local : resolveWorld(parent) * local;

    return worldMatrices[e] = world;
}

void RenderSystem::updateWorldMatrices() {
    worldMatrices.clear();
    for (const auto& [e, t] : scene.transforms)
        resolveWorld(e);
}

void RenderSystem::drawOutline() {
    const auto& camera = player.getCamera();

    for (auto& [e, worldComp] : reg.worlds) {

        // Warning: entity does not have shader, must use worldComp ids
        Shader* shader = reg.getShaderWithId(worldComp.shaderId);
        Shader* outlineShader = reg.getOutlineShaderWithId(worldComp.outlineShaderId);
        World& world = *reg.getWorldWithId(worldComp.worldId);

        if (player.getHasSelectedBlock() && outlineShader) {

            glm::vec3 selectedPos = player.getSelectedBlockPos();
            int selectedBlock = world.getBlock(selectedPos);
            int meshId = reg.getMeshId(selectedBlock);

            glm::mat4 targetModel = glm::translate(glm::mat4(1.0f), blockCenter(selectedPos));
            glm::mat4 outlineModel = glm::scale(targetModel, glm::vec3(1.05f));

            glDepthFunc(GL_LEQUAL);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
            glStencilMask(0xFF);

            // silhouette pass: stencil/depth only, never touch color
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            shader->use();
            shader->setMat4("view", camera.GetViewMatrix());
            shader->setMat4("projection", engineSettings.getProjectionMatrix());
            shader->setMat4("model", targetModel);
            drawMesh(meshId);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0x00);
            glDepthMask(GL_FALSE);

            outlineShader->use();
            outlineShader->setMat4("view", camera.GetViewMatrix());
            outlineShader->setMat4("projection", engineSettings.getProjectionMatrix());
            outlineShader->setMat4("model", outlineModel);
            outlineShader->setVec3("outlineColor", world.getOutlineColor());

            drawMesh(meshId);

            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glStencilMask(0xFF);
            glStencilFunc(GL_ALWAYS, 1, 0xFF);
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        }
    }
}

FrameData RenderSystem::buildFrameData() {
    const Camera& cam = player.getCamera();

    FrameData f;
    f.view       = cam.GetViewMatrix();
    f.projection = engineSettings.getProjectionMatrix();
    f.camPos     = cam.Position;
    return f;
}

void RenderSystem::bindFrameUniforms(Shader& s) {
    s.setMat4("view", frame.view);
    s.setMat4("projection", frame.projection);
    s.setVec3("viewPos", frame.camPos);
    s.setFloat("ambientStrength", lightSettings.ambientStrength);
    s.setVec3("ambientColor", lightSettings.ambientColor);
    uploadLights(s);
    uploadDirLights(s);
}

void RenderSystem::draw() {
    // apply debug settings
    // e.g. wireframe view
    applyDebugState();

    collectRenderItems();
    collectGpuLights(); 

    drawOpaquePass();
    drawTransparentPass();
    drawOutline();
}