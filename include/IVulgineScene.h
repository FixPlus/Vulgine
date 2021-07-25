//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_IVULGINESCENE_H
#define TEST_EXE_IVULGINESCENE_H

#include <../include/IVulgineObjects.h>
#include <vector>
namespace Vulgine{



    struct Scene: virtual public Object{
        std::vector<MeshRef> drawList{};

        virtual void createBackGround(const char* fragmentShaderModule = "frag_background"
                , std::vector<std::pair<DescriptorInfo, Descriptor>> const& descriptors = {}) = 0;
        virtual void deleteBackGround() = 0;

        virtual LightRef createLightSource() = 0;
        virtual CameraRef createCamera() = 0;


    };

    using SceneRef = SharedRef<Scene>;

}
#endif //TEST_EXE_IVULGINESCENE_H
