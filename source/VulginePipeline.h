//
// Created by Бушев Дмитрий on 05.07.2021.
//

#ifndef TEST_EXE_VULGINEPIPELINE_H
#define TEST_EXE_VULGINEPIPELINE_H


#include "VulgineObjects.h"
#include "VulgineScene.h"
#include "VulgineRenderPass.h"


namespace Vulgine{

    struct Pipeline: public Creatable{
        MaterialImpl* material;
        SceneImpl* scene;
        RenderPass* renderPass;

        VkPipeline pipeline;
        VkPipelineLayout pipelineLayout;

        Pipeline(MaterialImpl* mat, SceneImpl* scn, RenderPass* rp): material(mat), renderPass(rp), scene(scene){}
        void createImpl() override;
        void destroyImpl() override;

        ~Pipeline() override;
    };
}
#endif //TEST_EXE_VULGINEPIPELINE_H
