//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineScene.h"
#include "VulgineObjects.h"
#include "VulgineRenderPass.h"
#include "Utilities.h"
#include <algorithm>
#include "Vulgine.h"

namespace Vulgine{

    Mesh* SceneImpl::createEmptyMesh() {

        auto id = ObjectImpl::claimId();

        return &((meshes.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);
    }

    void SceneImpl::deleteMesh(Mesh *mesh) {

        meshes.erase(mesh->id());
    }

    Light *SceneImpl::createLightSource() {

        if(lightsInfo.lightCount == MAX_LIGHTS){
            Utilities::ExitFatal(-1, "Can't create more than " +  std::to_string(MAX_LIGHTS) + " light objects per scene");
        }

        auto id = ObjectImpl::claimId();

        lightMap.emplace(id, lightsInfo.lightCount);
        reverseLightMap.emplace(lightsInfo.lightCount, id);

        lightsInfo.lightCount++;

        return &((lights.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);

    }

    void SceneImpl::deleteLightSource(Light *light) {

        auto lightNumber = lightMap.at(light->id());
        lightMap.erase(light->id());
        for(auto i = lightNumber; i < lightsInfo.lightCount - 1; i++){
            reverseLightMap.at(i) = reverseLightMap.at(i + 1);
            lightMap[reverseLightMap.at(i)] = i;
        }
        lightsInfo.lightCount--;
        reverseLightMap.erase(lightsInfo.lightCount);

        lights.erase(light->id());
    }

    void SceneImpl::draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass* pass, int currentFrame) {

        // draw all the meshes

        for(auto& mesh: meshes)
            mesh.second.draw(commandBuffer, camera, pass, currentFrame);
    }

    Camera *SceneImpl::createCamera() {

        auto id = ObjectImpl::claimId();

        return &((cameras.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);
    }

    void SceneImpl::deleteCamera(Camera *camera) {
        cameras.erase(camera->id());
    }

    void SceneImpl::createImpl() {
        lightUBO.dynamic = true;
        lightUBO.size = sizeof(lightsInfo);
        lightUBO.pData = &lightsInfo;
        lightUBO.create();
        lightUBO.update();


    }

    void SceneImpl::destroyImpl() {
        lights.clear();
        cameras.clear();
        meshes.clear();
        lightUBO.destroy();

    }

    void SceneImpl::updateLight(uint32_t light) {
        auto lightNumber = lightMap.at(light);
        lightsInfo.lights[lightNumber].lightColor = glm::vec4{lights.at(light).color, 0.0f};
        glm::mat4 rotate = glm::rotate(glm::radians(lights.at(light).direction.x), glm::vec3{1.0f, 0.0f, 0.0f});
        rotate = glm::rotate(rotate,glm::radians(lights.at(light).direction.y), glm::vec3{0.0f, 1.0f, 0.0f});
        glm::vec4 direction = glm::vec4{1.0f, 0.0f, 0.0f, 0.0f};
        direction = rotate * direction;

        lightsInfo.lights[lightNumber].lightDirection = direction;
        lightUBO.update();
    }

    void SceneImpl::createBackGround(const char *fragmentShaderModule, const std::vector<DescriptorInfo> &descriptors) {
        if(background.created)
            return;

        background.created = true;
        background.material.custom = true;
        background.material.customMaterialInfo.descriptors = descriptors;
        background.material.customMaterialInfo.fragmentShader = fragmentShaderModule;
        background.material.customMaterialInfo.inputAttributes = {AttributeFormat::RGB32SF}; // inUVW

        background.material.setName(objectLabel() + " background");

        background.material.create();

    }

    void SceneImpl::deleteBackGround() {
        if(!background.created)
            return;

        background.created = false;
        background.material.destroy();
    }

    void SceneImpl::drawBackground(VkCommandBuffer commandBuffer, CameraImpl* camera, RenderPass* pass, int currentFrame) {
        // draw background

        if(background.created){
            auto& pipeline = GetImpl().pipelineMap.bind({nullptr, &background.material, this, dynamic_cast<RenderPassImpl*>(pass)}, commandBuffer);

            if(background.material.set.isCreated()){
                background.material.set.bind(0, commandBuffer, pipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);
            }
            vkCmdDraw(commandBuffer, 4, 1, 0, 0);
        }
    }


}