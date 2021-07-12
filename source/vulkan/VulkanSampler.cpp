//
// Created by Бушев Дмитрий on 11.07.2021.
//

#include "VulkanSampler.h"
#include "Vulgine.h"

void Vulgine::Sampler::create(VkSamplerAddressMode addressMode) {
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = addressMode;
    samplerCreateInfo.addressModeV = addressMode;
    samplerCreateInfo.addressModeW = addressMode;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0f;
    // Max level-of-detail should match mip level count
    samplerCreateInfo.maxLod = 0.0f;
    // Only enable anisotropic filtering if enabled on the device
    samplerCreateInfo.maxAnisotropy = vlg_instance->device->enabledFeatures.samplerAnisotropy ? vlg_instance->device->properties.limits.maxSamplerAnisotropy : 1.0f;
    samplerCreateInfo.anisotropyEnable = vlg_instance->device->enabledFeatures.samplerAnisotropy;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    VK_CHECK_RESULT(vkCreateSampler(vlg_instance->device->logicalDevice, &samplerCreateInfo, nullptr, &sampler));

}

void Vulgine::Sampler::destroy() {
    vkDestroySampler(vlg_instance->device->logicalDevice, sampler, nullptr);
    sampler = nullptr;
}