//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINERENDERPASS_H
#define TEST_EXE_VULGINERENDERPASS_H

#include "vulkan/vulkan.h"
#include "VulgineScene.h"
#include "VulgineFramebuffer.h"

namespace Vulgine{

    struct RenderPass{
        VkRenderPass renderPass = VK_NULL_HANDLE;

        virtual void buildCmdBuffers(VkCommandBuffer buffer, Framebuffer* framebuffer) = 0;

        virtual ~RenderPass();
    };


    struct DefaultRenderPass: public RenderPass{
        SceneImpl* scene;
        CameraImpl* camera;

        void buildCmdBuffers(VkCommandBuffer buffer, Framebuffer* framebuffer) override;
    };
}
#endif //TEST_EXE_VULGINERENDERPASS_H
