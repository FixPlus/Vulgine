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

        set.addUniformBuffer(&lightUBO, VK_SHADER_STAGE_FRAGMENT_BIT);
        set.pool = &vlg_instance->perScenePool;
        set.create();

    }

    void SceneImpl::destroyImpl() {
        lights.clear();
        cameras.clear();
        meshes.clear();
        lightUBO.destroy();
        set.destroy();
        set.clearDescriptors();
    }

    void SceneImpl::updateLight(uint32_t light) {
        auto lightNumber = lightMap.at(light);
        lightsInfo.lights[lightNumber].lightColor = glm::vec4{lights.at(light).color, 0.0f};
        lightsInfo.lights[lightNumber].lightDirection = glm::vec4{lights.at(light).direction, 0.0f};
        lightUBO.update();
    }


}