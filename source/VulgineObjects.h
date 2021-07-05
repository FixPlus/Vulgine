//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINEOBJECTS_H
#define TEST_EXE_VULGINEOBJECTS_H

#include <../include/IVulgineObjects.h>
#include <../include/IVulgineScene.h>
#include "Utilities.h"

#include <map>

namespace Vulgine{

    struct MeshImpl: public Mesh{

        explicit MeshImpl(Scene* parent, uint32_t id): Mesh(parent, id){ logger("Mesh created");};

        void createImpl() override;
        void destroyImpl() override;


        void updateVertexData() override;


        void updateInstanceData() override;


        ~MeshImpl() override{ logger("Mesh destroyed");/* TODO: destruction*/};
    };

    struct LightImpl: public Light{

        explicit LightImpl(Scene* parent, uint32_t id): Light(parent, id){};


        void createImpl() override;
        void destroyImpl() override;
        ~LightImpl() override{ /*TODO: disconnect from parent */};
    };

    struct ShaderModule: public Creatable{
        VkShaderModule module;
        std::string name;

        void createImpl() override;
        void destroyImpl() override;

        ~ShaderModule() override;
    };

    class MaterialImpl: public Material{

        VkVertexInputBindingDescription vertexAttrBindingInfo[2] = {};

        std::vector<VkVertexInputAttributeDescription> attributesDesc{};


    public:

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};

        void compileVertexInputState();

        // this vector's size exactly matches count of RenderPasses in VulgineImpl instance


        explicit MaterialImpl(uint32_t id): Material(id){}
        void createImpl() override;
        void destroyImpl() override;

        ~MaterialImpl() override;
    };

    struct CameraImpl: public Camera{

        CameraImpl(Scene* parent, uint32_t id): Camera(parent, id){logger("Camera created");}
        void createImpl() override;
        void destroyImpl() override;

        ~CameraImpl() override{ logger("Camera freed");}
    };

}
#endif //TEST_EXE_VULGINEOBJECTS_H
