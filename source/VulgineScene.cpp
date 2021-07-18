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

        auto id = ObjectImpl::claimId();

        return &((meshes.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);
    }

    void SceneImpl::deleteMesh(Mesh *mesh) {

        meshes.erase(mesh->id());
    }

    Light *SceneImpl::createLightSource() {

        auto id = ObjectImpl::claimId();

        return &((lights.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);

    }

    void SceneImpl::deleteLightSource(Light *light) {


        lights.erase(light->id());
    }

    void SceneImpl::draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass* pass) {
        for(auto& mesh: meshes)
            mesh.second.draw(commandBuffer, camera, pass);
    }

    Camera *SceneImpl::createCamera() {

        auto id = ObjectImpl::claimId();

        return &((cameras.emplace(std::piecewise_construct, std::forward_as_tuple(id), std::forward_as_tuple(this, id)).first)->second);
    }

    void SceneImpl::deleteCamera(Camera *camera) {
        cameras.erase(camera->id());
    }

    void SceneImpl::createImpl() {

    }

    void SceneImpl::destroyImpl() {
        lights.clear();
        cameras.clear();
        meshes.clear();
    }

}