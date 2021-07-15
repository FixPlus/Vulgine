//
// Created by Бушев Дмитрий on 11.07.2021.
//

#include "VulkanDescriptable.h"
#include "Vulgine.h"
#include "vulkan/VulkanInitializers.hpp"

void Vulgine::CombinedImageSampler::setupDescriptor() {
    descriptor.imageView = image->createImageView();
    descriptor.imageLayout = claimedLayout;
    descriptor.sampler = sampler.sampler;
}

void Vulgine::CombinedImageSampler::destroyDescriptor() {
    vkDestroyImageView(vlg_instance->device->logicalDevice, descriptor.imageView, nullptr);
}

Vulgine::CombinedImageSampler::CombinedImageSampler(Memory::Image *img) : DImage(img), Descriptable(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
    sampler.create();
}

Vulgine::CombinedImageSampler::~CombinedImageSampler() {
    sampler.destroy();
}

VkWriteDescriptorSet Vulgine::CombinedImageSampler::write(int binding) {
    VkWriteDescriptorSet ret = initializers::writeDescriptorSet(nullptr, descriptorType, binding, &descriptor);
    return ret;
}
