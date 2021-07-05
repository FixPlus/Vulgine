//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINESCENE_H
#define TEST_EXE_VULGINESCENE_H

#include <../include/IVulgineScene.h>
#include <VulgineObjects.h>
#include <stack>

#include <unordered_map>

namespace Vulgine{


    struct SceneImpl: public Scene{

        explicit SceneImpl(uint32_t id): Scene(id){};
        std::stack<uint32_t> meshFreeIds;
        std::stack<uint32_t> lightFreeIds;
        std::stack<uint32_t> cameraFreeIds;

        std::unordered_map<uint32_t, MeshImpl> meshes;
        std::unordered_map<uint32_t, LightImpl> lights;
        std::unordered_map<uint32_t, CameraImpl> cameras;

        Light* createLightSource() override;
        Mesh* createEmptyMesh() override;
        Camera* createCamera() override;

        void deleteMesh(Mesh* mesh) override;
        void deleteLightSource(Light* light) override;
        void deleteCamera(Camera* camera) override;

        void draw(CameraImpl* camera);

        ~SceneImpl() override{ logger("Scene deleted");}

    };
}
#endif //TEST_EXE_VULGINESCENE_H
