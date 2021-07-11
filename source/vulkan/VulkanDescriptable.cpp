//
// Created by Бушев Дмитрий on 11.07.2021.
//

#include "VulkanDescriptable.h"
#include "Vulgine.h"

void Vulgine::CombinedImageSampler::setupDescriptor() {
    descriptor.imageView = image->createImageView();
    descriptor.imageLayout = claimedLayout;
    descriptor.sampler = sampler->sampler;
}

void Vulgine::CombinedImageSampler::destroyDescriptor() {
    vkDestroyImageView(vlg_instance->device->logicalDevice, descriptor.imageView, nullptr);
}
