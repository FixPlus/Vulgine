//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINESCENE_H
#define TEST_EXE_IVULGINESCENE_H

#include <../include/IVulgineObjects.h>
#include <vector>
#include <memory>
namespace Vulgine{


    struct Scene{

        virtual Light* createLightSource() = 0;
        virtual Mesh* createEmptyMesh() = 0;

        virtual void disconnectMesh(Mesh* mesh) = 0;
        virtual void disconnectLightSource(Light* light) = 0;
        virtual ~Scene() = default;
    };

}
#endif //TEST_EXE_IVULGINESCENE_H
