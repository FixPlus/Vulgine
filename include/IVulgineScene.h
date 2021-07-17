//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINESCENE_H
#define TEST_EXE_IVULGINESCENE_H

#include <../include/IVulgineObjects.h>
#include <vector>
#include <memory>
namespace Vulgine{


    struct Scene: virtual public Object{


        virtual Light* createLightSource() = 0;
        virtual Mesh* createEmptyMesh() = 0;
        virtual Camera* createCamera() = 0;

        virtual void deleteMesh(Mesh* mesh) = 0;
        virtual void deleteLightSource(Light* light) = 0;
        virtual void deleteCamera(Camera* camera) = 0;

    };

}
#endif //TEST_EXE_IVULGINESCENE_H
