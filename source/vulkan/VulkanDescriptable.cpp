//
// Created by Бушев Дмитрий on 11.07.2021.
//

#include "VulkanDescriptable.h"
#include "Vulgine.h"
#include "vulkan/VulkanInitializers.hpp"

void Vulgine::CombinedImageSampler::setupDescriptor() {
    descriptor.imageView = image->createImageView();
    descriptor.imageLayout = claimedLayout;
    descriptor.sampler = sampler;
}

void Vulgine::CombinedImageSampler::destroyDescriptor() {
    vkDestroyImageView(GetImpl().device->logicalDevice, descriptor.imageView, nullptr);
}

Vulgine::CombinedImageSampler::CombinedImageSampler(Memory::Image *img, VkSampler sp) : DImage(img), sampler(sp),  Descriptable(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {

}

Vulgine::CombinedImageSampler::~CombinedImageSampler() {

}

VkWriteDescriptorSet Vulgine::CombinedImageSampler::write(int binding) {
    VkWriteDescriptorSet ret = initializers::writeDescriptorSet(VK_NULL_HANDLE, descriptorType, binding, &descriptor);
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
    VkWriteDescriptorSet ret = initializers::writeDescriptorSet(VK_NULL_HANDLE, descriptorType, binding, &descriptor);
    return ret;
}

Vulgine::DInputAttachment::DInputAttachment(VkImageView view) : DImage(nullptr), Descriptable(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
    descriptor.imageView = view;
}

VkWriteDescriptorSet Vulgine::DInputAttachment::write(int binding) {
    VkWriteDescriptorSet ret = initializers::writeDescriptorSet(VK_NULL_HANDLE, descriptorType, binding, &descriptor);
    return ret;
}

void Vulgine::DInputAttachment::setupDescriptor() {

    descriptor.imageLayout = claimedLayout;
    descriptor.sampler = VK_NULL_HANDLE;
}

void Vulgine::DInputAttachment::destroyDescriptor() {
}
