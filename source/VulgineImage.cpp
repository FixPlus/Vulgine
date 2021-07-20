//
// Created by Бушев Дмитрий on 11.07.2021.
//


#define STB_IMAGE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <cassert>

#include "VulgineImage.h"
#include "Utilities.h"
#include "Vulgine.h"

bool Vulgine::StaticImageImpl::loadFromFile(const char *filename, Image::FileFormat fileFormat) {
    assert(!image.allocated && "Image has been already allocated");
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if(pixels == nullptr){
        errs("Cannot read image data from file \'" + std::string(filename) + "\'");
        return false;
    }

    bool ret = loadFromPixelData(pixels, texWidth, texHeight, fileFormat);

    stbi_image_free(pixels);

    return ret;

}

bool Vulgine::StaticImageImpl::load(const unsigned char *data, uint32_t len, Image::FileFormat fileFormat) {
    assert(!image.allocated && "Image has been already allocated");
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load_from_memory(data, len, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if(pixels == nullptr){
        errs("Cannot read image data from address #" + std::to_string((unsigned long long)data));
        return false;
    }
    bool ret = loadFromPixelData(pixels, texWidth, texHeight, fileFormat);

    stbi_image_free(pixels);

    return ret;
}

bool Vulgine::StaticImageImpl::loadFromPixelData(const unsigned char *pixels, int texWidth, int texHeight, Image::FileFormat fileFormat) {
    uint32_t imageSize = texWidth * texHeight * 4;

    Memory::StagingBuffer stagingBuffer;

    stagingBuffer.create(imageSize);
    stagingBuffer.fill(pixels);


    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    image.allocate(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    image.transitImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    image.copyFromBuffer(stagingBuffer.buffer, texWidth, texHeight);

    image.transitImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vlg_instance->gui.addTexturedImage(&image);
    return true;
}

Vulgine::StaticImageImpl::~StaticImageImpl() {
    vlg_instance->gui.deleteTexturedImage(&image);
    if(image.allocated)
        image.free();
}

void Vulgine::StaticImageImpl::createImpl() {

}

void Vulgine::StaticImageImpl::destroyImpl() {

}

void Vulgine::DynamicImageImpl::createImpl() {
    auto imageCount = vlg_instance->swapChain.imageCount;
    images.reserve(imageCount);
    for(int i = 0; i < imageCount; ++i){
        auto& image = images.emplace_back();
        image.allocate(createInfo, VMA_MEMORY_USAGE_GPU_ONLY, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        image.transitImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

void Vulgine::DynamicImageImpl::destroyImpl() {
    images.clear();
}

