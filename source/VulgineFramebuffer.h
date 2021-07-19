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
    protected:
        void createImpl() override;
        void destroyImpl() override;
    public:


        std::unordered_map<uint32_t, DynamicImageImpl> attachmentsImages;
        std::vector<std::vector<VkImageView>> attachments;

        Image* addAttachment(Type type) override;

        void addOnsrceenAttachment(std::vector<VkImageView> const& views);

        uint32_t attachmentCount() override;

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
