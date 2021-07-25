//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINERENDERPASS_H
#define TEST_EXE_VULGINERENDERPASS_H

#include "vulkan/vulkan.h"
#include "VulgineFramebuffer.h"
#include "VulginePipeline.h"
namespace Vulgine{

    struct RenderPassImpl: public RenderPass, public ObjectImplNoMove{

        std::vector<VkClearValue> clearValues;
        struct DeferredLighting{

            // specialized pipeline for deferred lighting

            CompositionPipeline pipeline;

            // contains input attachment (G-buffer) and light ubos/textures etc. descriptors layout

            DescriptorSet compositionSet;

            // contain input attachment (Color target) and light ubos/textures etc. descriptor layout

            DescriptorSet transparentSet;
        };


        /** enabled only if scene contains at least one light. Otherwise default forward rendering will be used.
         *
         * If enabled, 3 additinal color render targets (G-buffer) will be implicitly allocated along with depth
         * target if missing one. This render targets are fully transient within this render pass, so they must not be
         * exposed via Vulgine interface. When deferred rendering is used, adding arbitrary render targets is not
         * allowed. Attachment layout for geometry subpass will look like this:
         *
         *
         *  subpass 0: (Solid geometry pass)
         *      out
         *        0: albedo attachment
         *        1: world-normal attachment
         *        2: world-position attachment
         *        3: depth-buffer attachment
         *  subpass 1: (Deferred lighting pass)
         *      in
         *        0: albedo attachment
         *        1: world-normal attachment
         *        2: world-position attachment
         *      out
         *        0: composite-color attachment
         *   subpass 2: (Transparent geometry pass)
         *       in
         *        0: composite-color attachment
         *        1: depth-buffer attachment
         *       out:
         *        0: composite-color attachment
         *        1: depth-buffer attachment
         *
         * */
        DeferredLighting deferredLightingSubpass;
        bool deferredEnabled = false;

        void createFramebuffer();
        void destroyFramebuffer();




        explicit RenderPassImpl(uint32_t id): ObjectImplNoMove(Object::Type::RENDER_PASS, id){};

        FrameBufferImpl frameBuffer{};

        VkRenderPass renderPass = VK_NULL_HANDLE;

        ImageRef addAttachment(AttachmentType type = AttachmentType::COLOR) override;

        uint32_t attachmentCount() override;

        ImageRef getAttachment(uint32_t binding) override;

        void initFrameBuffer(SharedRef<RenderPassImpl> const& thisRef);

        void buildPass();

        void begin(VkCommandBuffer buffer, int currentFrame);

        void buildCmdBuffers(VkCommandBuffer buffer, int currentFrame);

        void end(VkCommandBuffer buffer);

        ~RenderPassImpl() override;
    protected:
        void createImpl() override;
        void destroyImpl() override;

    };

    using RenderPassImplRef = SharedRef<RenderPassImpl>;
}
#endif //TEST_EXE_VULGINERENDERPASS_H
