//
// Created by Бушев Дмитрий on 05.07.2021.
//

#ifndef TEST_EXE_VULGINEPIPELINE_H
#define TEST_EXE_VULGINEPIPELINE_H


#include "VulgineObjects.h"
#include "VulgineScene.h"
#include "VulgineRenderPass.h"


namespace Vulgine{

    struct PipelineKey{
        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        MaterialImpl* material = nullptr;
        SceneImpl* scene = nullptr;
        RenderPass* renderPass = nullptr;

        bool operator<(PipelineKey const& another) const;

    };

    struct Pipeline: public Creatable{
        MaterialImpl* material;
        SceneImpl* scene;
        RenderPass* renderPass;
        VkPipelineVertexInputStateCreateInfo vertexFormat;

        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        explicit Pipeline(PipelineKey key = {}):
                            vertexFormat(key.vertexInputState), material(key.material), renderPass(key.renderPass), scene(key.scene){};
        Pipeline& operator=(Pipeline&& another) = default;
        void createImpl() override;
        void destroyImpl() override;

        void bind(VkCommandBuffer cmdBuffer) const;

        ~Pipeline() override;
    };
}
#endif //TEST_EXE_VULGINEPIPELINE_H
