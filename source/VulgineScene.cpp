//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineScene.h"
#include "VulgineObjects.h"
#include "Utilities.h"
#include <algorithm>


namespace Vulgine{

    Mesh* SceneImpl::createEmptyMesh() {

        MeshImpl* ret;

        try{
            ret = new MeshImpl{this};
        }
        catch (std::bad_alloc const& e){
            Utilities::ExitFatal(-1,"Memory allocation failed");
        }

        meshes.emplace_back(ret);

        return ret;
    }

    void SceneImpl::disconnectMesh(Mesh *mesh) {
        meshes.erase(std::remove_if(meshes.begin(), meshes.end(), [mesh](Mesh* elem){ return elem == mesh;}), meshes.end());
    }

    Light *SceneImpl::createLightSource() {

        LightImpl* ret;

        try{
            ret = new LightImpl{this};
        }
        catch (std::bad_alloc const& e){
            Utilities::ExitFatal(-1,"Memory allocation failed");
        }

        lights.emplace_back(ret);

        return ret;
    }

    void SceneImpl::disconnectLightSource(Light *light) {
        lights.erase(std::remove_if(lights.begin(), lights.end(), [light](Light* elem){ return elem == light;}), lights.end());
    }

    void SceneImpl::draw(CameraImpl *camera) {

    }

}