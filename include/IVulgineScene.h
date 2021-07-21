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


        virtual void createBackGround(const char* fragmentShaderModule = "frag_background"
                , std::vector<DescriptorInfo> const& descriptors = {}) = 0;
        virtual void deleteBackGround() = 0;

        virtual Light* createLightSource() = 0;
        virtual Mesh* createEmptyMesh() = 0;
        virtual Camera* createCamera() = 0;

        virtual void deleteMesh(Mesh* mesh) = 0;
        virtual void deleteLightSource(Light* light) = 0;
        virtual void deleteCamera(Camera* camera) = 0;

    };

}
#endif //TEST_EXE_IVULGINESCENE_H
