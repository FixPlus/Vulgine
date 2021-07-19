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
        MeshImpl* mesh = nullptr;
        MaterialImpl* material = nullptr;
        SceneImpl* scene = nullptr;
        RenderPassImpl* renderPass = nullptr;


        bool operator<(PipelineKey const& another) const;

    };

    struct Pipeline: public ObjectImpl{
        MaterialImpl* material;
        SceneImpl* scene;
        RenderPassImpl* renderPass;
        MeshImpl* mesh;

        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        explicit Pipeline(PipelineKey key = {}): ObjectImpl(Object::Type::PIPELINE, ObjectImpl::claimId()),
                            mesh(key.mesh), material(key.material), renderPass(key.renderPass), scene(key.scene){};
        Pipeline& operator=(Pipeline&& another) = default;
        void createImpl() override;
        void destroyImpl() override;

        void bind(VkCommandBuffer cmdBuffer) const;

        ~Pipeline() override;
    };
}
#endif //TEST_EXE_VULGINEPIPELINE_H
