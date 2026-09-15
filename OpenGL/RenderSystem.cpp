#include "RenderSystem.h"
#include "IRenderable.h"

#include <algorithm>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>   // glm::distance2

void RenderSystem::render(RenderItem renderable, int s) {

    Shader* shader;
    int e = renderable.e;

    // if shader is not given, use shader assigned to entity
    if (!s) {
        shader = reg.getShader(e);
    } else {
        shader = reg.getShaderWithId(s);
    }

    const Mesh* mesh = reg.getMesh(e);
    const auto& camera = player.getCamera();
    const glm::mat4& model = renderable.model;
    const MaterialComponent& mat = reg.materials.at(e);

    shader->use();

    shader->setFloat("ambientStrength", lightSettings.ambientStrength);
    shader->setVec3("ambientColor", lightSettings.ambientColor);
    shader->setMat4("model", model);
    shader->setMat4("view", camera.GetViewMatrix());
    shader->setMat4("projection", engineSettings.getProjectionMatrix());
    shader->setVec3("viewPos", camera.Position);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);
    shader->setInt("material.diffuse", 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mat.specularTexture);
    shader->setInt("material.specular", 1);

    shader->setVec3("material.color", mat.color);
    shader->setFloat("material.shininess", mat.shininess);
    shader->setFloat("material.emissive", mat.emissive);
    shader->setFloat("material.alphaCutoff", mat.alphaCutoff);
    shader->setFloat("material.alpha", mat.alpha);
    shader->setBool("material.isMasked", mat.isMasked);

    uploadLights(reg, *shader);
    uploadDirLights(reg, *shader, lightSettings.dirLightEnabled);

    drawMesh(reg.getMeshId(e));
}

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

        uploadLights(reg, *s);
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

void RenderSystem::drawMesh(int meshId) {

    Mesh* mesh = reg.getMeshWithId(meshId);

    glBindVertexArray(mesh->VAO);
    glDisableVertexAttribArray(3);   // don't let non-instanced draws read leftover aInstancePos
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderSystem::drawMeshInstanced(Mesh* mesh, int count) {
    glBindVertexArray(mesh->VAO);
    glDrawElementsInstanced(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0, count);
    glBindVertexArray(0);
}

void RenderSystem::uploadLights(const Registry& reg, Shader& shader) {
    shader.setBool("pointLightsEnabled", lightSettings.pointLightsEnabled);
    shader.setInt("numPointLights", (int)reg.pointLights.size());

    int i = 0;
    for (const auto& [e, pointLightComp] : reg.pointLights) {
        auto worldIt = reg.worldMatrices.find(e);
        if (worldIt == reg.worldMatrices.end()) continue;
        glm::vec3 lightPos = glm::vec3(worldIt->second.value[3]); // world-space position

        std::string base = "pointLights[" + std::to_string(i) + "].";
        shader.setVec3(base + "position", lightPos);
        shader.setVec3(base + "color", pointLightComp.color);
        shader.setFloat(base + "intensity", pointLightComp.intensity);
        shader.setFloat(base + "constant", pointLightComp.constant);
        shader.setFloat(base + "linear", pointLightComp.linear);
        shader.setFloat(base + "quadratic", pointLightComp.quadratic);
        i++;
    }
}

void RenderSystem::uploadDirLights(const Registry& reg, Shader& shader, bool enabled) {
    if (!enabled) { shader.setInt("numDirLights", 0); return; }

    shader.setInt("numDirLights", (int)reg.dirLights.size());

    int i = 0;
    for (const auto& [e, dirLightComp] : reg.dirLights) {
        glm::vec3 dir = dirLightComp.direction;
        std::string base = "dirLights[" + std::to_string(i) + "].";
        shader.setVec3(base + "direction", dir);
        shader.setVec3(base + "color", dirLightComp.color);
        shader.setFloat(base + "intensity", dirLightComp.intensity);
        i++;
    }
}

void RenderSystem::collectRenderItems() {
    opaque.clear();
    transparent.clear();
    instancedOpaque.clear();

    // entities
    for (int e : reg.renderableEntities) {
        const MaterialComponent& mat = reg.materials.at(e);
        const glm::mat4& model = reg.worldMatrices.at(e).value;

        if (mat.isTransparent) transparent.push_back({ e, model });
        else opaque.push_back({ e, model });
    }

    for (int e : reg.renderableWorlds) {
        World& world = *reg.getWorld(e);
        world.collectInstancedRenderItems(instancedOpaque, glm::mat4(1.0f));
        world.collectRenderItems(transparent, glm::mat4(1.0f));
    }
}

void RenderSystem::renderFrame() {
    // collect all renderable entities
    collectRenderItems();

    const glm::vec3 camPos = player.getCamera().Position;

    // --- opaque pass ---
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    //std::cout << "[RenderSystem] opaque batches: " << opaque.size() << std::endl;
    for (const auto& batch : opaque)
        //std::cout << "[RenderSystem]   e=" << batch.e
        //<< " positions=" << batch.positions.size() << std::endl;

    if (opaque.empty())
        std::cout << "[RenderSystem] WARNING: opaque list is empty, nothing will be drawn this pass\n";

    for (const auto& item : opaque)
        render(item);

    for (int e : reg.renderableWorlds) {
        int worldShaderId = reg.worlds.at(e).shaderId;
        renderInstanced(instancedOpaque, worldShaderId);
    }
    // --- transparent pass ---
    //std::cout << "[RenderSystem] transparent items: " << transparent.size() << std::endl;

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

    drawOutline();
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