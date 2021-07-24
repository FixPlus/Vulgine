//
// Created by Бушев Дмитрий on 05.07.2021.
//

#ifndef TEST_EXE_VULGINEPIPELINE_H
#define TEST_EXE_VULGINEPIPELINE_H


#include "VulgineObjects.h"
#include "VulgineScene.h"


namespace Vulgine{

    class RenderPassImpl;

    struct PipelineKey{
        GeometryImpl* geometry = nullptr;
        MaterialImpl* material = nullptr;
        SceneImpl* scene = nullptr;
        RenderPassImpl* renderPass = nullptr;


        bool operator<(PipelineKey const& another) const;

    };

    struct Pipeline: public ObjectImpl{

        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        Pipeline(): ObjectImpl(Object::Type::PIPELINE, ObjectImpl::claimId()){}

        virtual void bind(VkCommandBuffer cmdBuffer) const = 0;
    };

    /** TODO: make mid-layer class GraphicsPipeline derived from Pipeline class and make CompositionPipeline and
    *   TODO: GeneralPipeline it's children
     *
    */
    struct CompositionPipeline: public Pipeline{
        RenderPassImpl* renderPass = nullptr;
        CompositionPipeline& operator=(CompositionPipeline&& another) = default;
        void createImpl() override;
        void destroyImpl() override;

        void bind(VkCommandBuffer cmdBuffer) const override;

        ~CompositionPipeline() override;
    };

    struct TransparentPipeline: public Pipeline{

    };

    /**
     * @brief: General Pipeline is used as back-end pipeline for user-defined Geometry Subpass
     * (that involves rendering a Mesh(owned by a Scene) geometry using Material in specific render pass)
     *
     *
     */

    struct GeneralPipeline: public Pipeline{
        GeometryImpl* geometry;
        SceneImpl* scene;
        RenderPassImpl* renderPass;
        MaterialImpl* material;


        explicit GeneralPipeline(PipelineKey key = {}):
                geometry(key.geometry), material(key.material), renderPass(key.renderPass), scene(key.scene){};
        GeneralPipeline& operator=(GeneralPipeline&& another) = default;
        void createImpl() override;
        void destroyImpl() override;

        void bind(VkCommandBuffer cmdBuffer) const override;

        ~GeneralPipeline() override;
    };
}
#endif //TEST_EXE_VULGINEPIPELINE_H
