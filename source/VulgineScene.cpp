//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineScene.h"
#include "VulgineObjects.h"
#include "VulgineRenderPass.h"
#include "Utilities.h"
#include <algorithm>


namespace Vulgine{

    Mesh* SceneImpl::createEmptyMesh() {

        uint32_t id;
        if(!meshFreeIds.empty()) {
            id = meshFreeIds.top();
            meshFreeIds.pop();
        } else{
            id = meshes.size();
        }

        return &((meshes.emplace(std::piecewise_construct,std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);
    }

    void SceneImpl::deleteMesh(Mesh *mesh) {
        auto id = mesh->id();

        if(id != meshes.size() - 1)
            meshFreeIds.push(id);

        meshes.erase(id);
    }

    Light *SceneImpl::createLightSource() {

        uint32_t id;
        if(!lightFreeIds.empty()) {
            id = lightFreeIds.top();
            lightFreeIds.pop();
        } else{
            id = lights.size();
        }

        return &((lights.emplace(std::piecewise_construct,std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);

    }

    void SceneImpl::deleteLightSource(Light *light) {
        auto id = light->id();

        if(id != lights.size() - 1)
            lightFreeIds.push(id);

        lights.erase(id);
    }

    void SceneImpl::draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass* pass) {
        for(auto& mesh: meshes)
            mesh.second.draw(commandBuffer, camera, pass);
    }

    Camera *SceneImpl::createCamera() {

        uint32_t id;
        if(!cameraFreeIds.empty()) {
            id = cameraFreeIds.top();
            cameraFreeIds.pop();
        } else{
            id =cameras.size();
        }

        return &((cameras.emplace(std::piecewise_construct,std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);
    }

    void SceneImpl::deleteCamera(Camera *camera) {
        auto id = camera->id();

        if(id != cameras.size() - 1)
            cameraFreeIds.push(id);

        cameras.erase(id);
    }

}