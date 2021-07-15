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

Vulgine::DUniformBuffer::DUniformBuffer(Memory::Buffer* buf): buffer(buf), Descriptable(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {

}

void Vulgine::DUniformBuffer::setupDescriptor() {
    descriptor.buffer = buffer->buffer;
    descriptor.offset = 0;
    descriptor.range = VK_WHOLE_SIZE;
}

void Vulgine::DUniformBuffer::destroyDescriptor() {

}

VkWriteDescriptorSet Vulgine::DUniformBuffer::write(int binding) {
    VkWriteDescriptorSet ret = initializers::writeDescriptorSet(nullptr, descriptorType, binding, &descriptor);
    return ret;
}
