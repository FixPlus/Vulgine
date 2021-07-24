//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINEFRAMEBUFFER_H
#define TEST_EXE_VULGINEFRAMEBUFFER_H

#include <vulkan/vulkan.h>
#include "VulgineObject.h"
#include "VulgineImage.h"
#include <vector>

namespace Vulgine{
    class RenderPassImpl;

    class FrameBufferImpl: public FrameBuffer, public ObjectImplNoMove{
        bool hasDepth = false;
    protected:
        void createImpl() override;
        void destroyImpl() override;
    public:
        Image* createAttachment(VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);


        static const VkFormat GBufferAlbedoFormat = VK_FORMAT_R8G8B8A8_UNORM;
        static const VkFormat GBufferPosFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
        static const VkFormat GBufferNormFormat = GBufferPosFormat;

        static const VkFormat ColorBufferFormat = VK_FORMAT_R8G8B8A8_SRGB;

        std::unordered_map<uint32_t, DynamicImageImpl> attachmentsImages;
        std::vector<std::vector<VkImageView>> attachments;

        Image* addAttachment(Type type) override;

        void addOnsrceenAttachment(std::vector<VkImageView> const& views);
        void createGBuffer();

        uint32_t attachmentCount() override;
        uint32_t colorAttachmentCount();

        Image* getAttachment(uint32_t binding) override;


        FrameBufferImpl(): ObjectImplNoMove(Object::Type::FRAME_BUFFER, ObjectImpl::claimId()){};
        FrameBufferImpl(FrameBufferImpl&& another) = default;
        FrameBufferImpl& operator=(FrameBufferImpl&& another) = default;

        std::vector<VkFramebuffer> framebuffers;

        RenderPassImpl* renderPass = nullptr;


        ~FrameBufferImpl() override;
    };
}
#endif //TEST_EXE_VULGINEFRAMEBUFFER_H
