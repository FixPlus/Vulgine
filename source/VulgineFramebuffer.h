//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINEFRAMEBUFFER_H
#define TEST_EXE_VULGINEFRAMEBUFFER_H

#include <vulkan/vulkan.h>
#include "VulgineObject.h"
#include <vector>

namespace Vulgine{

    class Framebuffer: public ObjectImpl{
    protected:
        void createImpl() override;
        void destroyImpl() override;
    public:
        Framebuffer(uint32_t id = 0): ObjectImpl(id, Object::Type::FRAME_BUFFER){};
        Framebuffer(Framebuffer&& another) = default;
        Framebuffer& operator=(Framebuffer&& another) = default;
        VkFramebuffer framebuffer;

        int width, height;
        std::vector<VkImageView> attachments;
        VkRenderPass renderPass;


        ~Framebuffer() override;
    };
}
#endif //TEST_EXE_VULGINEFRAMEBUFFER_H
