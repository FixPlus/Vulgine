//
// Created by Бушев Дмитрий on 11.07.2021.
//

#ifndef TEST_EXE_VULKANDESCRIPTABLE_H
#define TEST_EXE_VULKANDESCRIPTABLE_H

#include <vulkan/vulkan.h>
#include "VulkanAllocatable.h"
#include "VulkanSampler.h"

namespace Vulgine{


    struct Descriptable{
        VkDescriptorType descriptorType;
        explicit Descriptable(VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM): descriptorType(type){}
        virtual VkWriteDescriptorSet write(int binding) = 0;
        virtual void setupDescriptor() = 0;
        virtual void destroyDescriptor() = 0;
        virtual ~Descriptable() = default;
    };

    struct DImage: public virtual Descriptable{
        Memory::Image* image;
        VkImageLayout claimedLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkDescriptorImageInfo descriptor{};
        explicit DImage(Memory::Image* img): image(img) { };
    };



    struct CombinedImageSampler: public DImage{
        Sampler sampler{};
        explicit CombinedImageSampler(Memory::Image* img);
        VkWriteDescriptorSet write(int binding) override;
        void setupDescriptor() override;
        void destroyDescriptor() override;
        ~CombinedImageSampler() override;
    };

    struct DBuffer: public virtual Descriptable, public virtual Memory::Buffer{
        VkDescriptorBufferInfo descriptor{};
        void setupDescriptor() override;
    };
}
#endif //TEST_EXE_VULKANDESCRIPTABLE_H
