//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINESCENE_H
#define TEST_EXE_VULGINESCENE_H

#include <../include/IVulgineScene.h>
#include <VulgineObjects.h>

#include <deque>

namespace Vulgine{


    struct SceneImpl: public Scene{
        std::deque<MeshImpl*> meshes;
        std::deque<LightImpl*> lights;

        Light* createLightSource() override;
        Mesh* createEmptyMesh() override;

        void disconnectMesh(Mesh* mesh) override;
        void disconnectLightSource(Light* light) override;

        void draw(CameraImpl* camera);

        ~SceneImpl() override{
            for(auto* mesh: meshes)
                dynamic_cast<MeshImpl*>(mesh)->disconnectFromParent();
        }
    };
}
#endif //TEST_EXE_VULGINESCENE_H
