//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINEOBJECTS_H
#define TEST_EXE_VULGINEOBJECTS_H

#include <../include/IVulgineObjects.h>
#include <../include/IVulgineScene.h>

namespace Vulgine{


    struct MeshImpl: public Mesh{

        explicit MeshImpl(Scene* parent): Mesh(parent){};

        void createImpl() override;
        void destroyImpl() override;


        void updateVertexData() override;


        void updateInstanceData() override;

        void disconnectFromParent() {
            if(parent()){
                parent_ = nullptr;
            }
        };

        ~MeshImpl() override{ if(parent()) parent()->disconnectMesh(this);};
    };

    struct LightImpl: public Light{

        explicit LightImpl(Scene* parent): Light(parent){};

        void disconnectFromParent() {
            if(parent()){
                parent_ = nullptr;
            }
        };

        void createImpl() override;
        void destroyImpl() override;
        ~LightImpl() override{ if(parent()) parent()->disconnectLightSource(this);};
    };

    struct RenderTargetImpl: public RenderTarget{
        void createImpl() override;
        void destroyImpl() override;
    };

    struct MaterialImpl: public Material{
        void createImpl() override;
        void destroyImpl() override;
    };

    struct CameraImpl: public Camera{
        void createImpl() override;
        void destroyImpl() override;
    };

}
#endif //TEST_EXE_VULGINEOBJECTS_H
