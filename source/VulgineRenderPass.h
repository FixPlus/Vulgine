//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINERENDERPASS_H
#define TEST_EXE_VULGINERENDERPASS_H

#include "vulkan/vulkan.h"
#include "VulgineFramebuffer.h"

namespace Vulgine{

    struct RenderPassImpl: public RenderPass, public ObjectImplNoMove{

        explicit RenderPassImpl(uint32_t id): ObjectImplNoMove(Object::Type::RENDER_PASS, id){};

        FrameBufferImpl frameBuffer{};

        VkRenderPass renderPass = VK_NULL_HANDLE;

        FrameBuffer* getFrameBuffer() override;

        void begin(VkCommandBuffer buffer, int currentFrame);

        void buildCmdBuffers(VkCommandBuffer buffer, int currentFrame);

        void end(VkCommandBuffer buffer);

        ~RenderPassImpl() override;
    protected:
        void createImpl() override;
        void destroyImpl() override;

    };
}
#endif //TEST_EXE_VULGINERENDERPASS_H
