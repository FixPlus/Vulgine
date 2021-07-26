//
// Created by Бушев Дмитрий on 07.07.2021.
//

#include "VulkanAllocatable.h"
#include "Vulgine.h"
#include "VulkanInitializers.hpp"



void Vulgine::Memory::Buffer::allocate(VkBufferCreateInfo bufferCI, VmaMemoryUsage memoryUsageFlags, VmaAllocationCreateFlags allocFlags, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags prefFlags) {
    if(allocated){
        vmaDestroyBuffer(GetImpl().allocator, buffer, allocation);
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageFlags;
    allocInfo.preferredFlags = prefFlags;
    allocInfo.requiredFlags = reqFlags;
    allocInfo.flags = allocFlags;


    vmaCreateBuffer(GetImpl().allocator, &bufferCI, &allocInfo, &buffer, &allocation, nullptr);

    allocated = true;
}

Vulgine::Memory::Buffer::~Buffer() {
    if(allocated){
        vmaDestroyBuffer(GetImpl().allocator, buffer, allocation);
    }
}

void Vulgine::Memory::Buffer::free() {
    if(allocated){
        vmaDestroyBuffer(GetImpl().allocator, buffer, allocation);
    }
    allocated = false;

    buffer = VK_NULL_HANDLE;
}

void Vulgine::Memory::Image::allocate(VkImageCreateInfo imageCI, VmaMemoryUsage memoryUsageFlags, VmaAllocationCreateFlags allocFlags, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags prefFlags) {
    if(allocated){
        free();
    }

    imageInfo = imageCI;
    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageFlags;
    allocInfo.preferredFlags = prefFlags;
    allocInfo.requiredFlags = reqFlags;
    allocInfo.flags = allocFlags;

    VK_CHECK_RESULT(vmaCreateImage(GetImpl().allocator, &imageCI, &allocInfo, &image, &allocation, nullptr))

    allocated = true;

}

void Vulgine::Memory::Image::free() {
    if(allocated){
        vmaDestroyImage(GetImpl().allocator, image, allocation);
    }

    allocated = false;

    image = VK_NULL_HANDLE;
}

Vulgine::Memory::Image::~Image() {
    if(allocated){
        vmaDestroyImage(GetImpl().allocator, image, allocation);
    }
}

namespace Vulgine {
    VkImageAspectFlagBits getAspect(VkFormat format) {
        if (format == GetImpl().depthFormat){
            int ret;
            ret = VK_IMAGE_ASPECT_DEPTH_BIT;
            if(format >= VK_FORMAT_D16_UNORM_S8_UINT){
                ret |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
            return static_cast<VkImageAspectFlagBits>(ret);
        } else{
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }
}
VkImageView Vulgine::Memory::Image::createImageView() const {
    VkImageView view;
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    switch (imageInfo.imageType) {
        case VK_IMAGE_TYPE_1D: viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D; break;
        case VK_IMAGE_TYPE_2D: viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; break;
        case VK_IMAGE_TYPE_3D: viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D; break;
        default: assert(0 && "Image Info is broken");
    }

    viewCreateInfo.format = imageInfo.format;
    viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
    viewCreateInfo.subresourceRange = { getAspect(imageInfo.format), 0, 1, 0, 1 };
    // Linear tiling usually won't support mip maps
    // Only set mip map count if optimal tiling is used
    viewCreateInfo.subresourceRange.levelCount = 1;
    viewCreateInfo.image = image;
    VK_CHECK_RESULT(vkCreateImageView(GetImpl().device->logicalDevice, &viewCreateInfo, nullptr, &view));

    return view;
}

void Vulgine::Memory::Image::transitImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer transitCmd = GetImpl().device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = getAspect(imageInfo.format);
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    } else {
        assert(0 && "This image transition layout isn't supported yet");
    }

    vkCmdPipelineBarrier(
            transitCmd,
            sourceStage /* TODO */, destinationStage /* TODO */,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
    );

    GetImpl().device->flushCommandBuffer(transitCmd, GetImpl().transferQueue, true);

}

void Vulgine::Memory::Image::copyFromBuffer(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t depth) {
    VkCommandBuffer copyCmd = GetImpl().device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
            width,
            height,
            depth
    };

    vkCmdCopyBufferToImage(
            copyCmd,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
    );

    GetImpl().device->flushCommandBuffer(copyCmd, GetImpl().transferQueue, true);

}

void Vulgine::Memory::ImmutableBuffer::create(void *pData, size_t size, VkBufferUsageFlagBits usage) {

    assert(pData && size && "Invalid data description");

    //vkQueueWaitIdle(GetImpl().transferQueue);

    if(allocated)
        free();

    // We will use staging buffer to perform direct memory mapping and the copy it to device local memory

    Buffer stagingBuffer;

    VkBufferCreateInfo bufferCI = initializers::bufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    stagingBuffer.allocate(bufferCI, VMA_MEMORY_USAGE_CPU_ONLY);

    bufferCI.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;


    // Need to explicitly state that I want device local memory because VMA_MEMORY_USAGE_GPU_ONLY doesn't guarantee it
    // to be so.

    allocate(bufferCI, VMA_MEMORY_USAGE_GPU_ONLY, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    void* mappedData;

    vmaMapMemory(GetImpl().allocator, stagingBuffer.allocation, &mappedData);

    memcpy(mappedData, pData, size);

    vmaUnmapMemory(GetImpl().allocator, stagingBuffer.allocation);


    // Copy from staging buffers

    VkCommandBuffer copyCmd = GetImpl().device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferCopy copyRegion = {};

    copyRegion.size = size;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, buffer, 1, &copyRegion);

    GetImpl().device->flushCommandBuffer(copyCmd, GetImpl().transferQueue, true);

    //vkQueueWaitIdle(GetImpl().transferQueue);
}

void Vulgine::Memory::ImmutableBuffer::free() {
    Buffer::free();
}

void Vulgine::Memory::VertexBuffer::bind(VkCommandBuffer cmdBuffer) {

    assert(allocated && "Trying to bind unallocated vertex buffer");

    const VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(cmdBuffer, binding, 1, &buffer, &offset);
}


void Vulgine::Memory::DynamicBuffer::create(size_t size, VkBufferUsageFlagBits usage) {

    assert(size && "Invalid data description");

    if(allocated) {
        vmaUnmapMemory(GetImpl().allocator, allocation);
        free();
    }

    dataSize = size;

    VkBufferCreateInfo bufferCI = initializers::bufferCreateInfo(usage, size);
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    allocate(bufferCI, VMA_MEMORY_USAGE_CPU_TO_GPU);

    vmaMapMemory(GetImpl().allocator, allocation, &mapped);
}

Vulgine::Memory::DynamicBuffer::~DynamicBuffer() {
    if(allocated)
        vmaUnmapMemory(GetImpl().allocator, allocation);
}

void Vulgine::Memory::DynamicBuffer::push(void* data, size_t size, size_t offset) {
    assert(allocated && "Memory not allocated for this buffer yet");
    if(size + offset > dataSize)
        Utilities::ExitFatal(-1, "Memcpy off bounds of mapped gpu memory");

    if(size == 0)
        size = dataSize;

    memcpy((char*)mapped + offset, data, size);
}

void Vulgine::Memory::DynamicBuffer::free() {
    vmaUnmapMemory(GetImpl().allocator, allocation);
    Buffer::free();
}

void Vulgine::Memory::StaticVertexBuffer::create(void *pData, size_t size) {
    ImmutableBuffer::create(pData, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void Vulgine::Memory::DynamicVertexBuffer::create(size_t size) {
    DynamicBuffer::create(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void Vulgine::Memory::StaticIndexBuffer::create(uint32_t *pData, size_t size) {
    ImmutableBuffer::create((void*)pData, size * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void Vulgine::Memory::DynamicIndexBuffer::create(size_t size) {
    DynamicBuffer::create(size * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}


void Vulgine::Memory::IndexBuffer::bind(VkCommandBuffer cmdBuffer) {
    vkCmdBindIndexBuffer(cmdBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void Vulgine::Memory::StagingBuffer::create(uint32_t size) {
    this->size = size;
    VkBufferCreateInfo bufferCI = initializers::bufferCreateInfo(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size);
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    allocate(bufferCI, VMA_MEMORY_USAGE_CPU_ONLY);

    vmaMapMemory(GetImpl().allocator, allocation, &mapped);
}

void Vulgine::Memory::StagingBuffer::fill(const void *data) {


    memcpy(mapped, data, size);


}

Vulgine::Memory::StagingBuffer::~StagingBuffer() {
    if(allocated) {
        vmaUnmapMemory(GetImpl().allocator, allocation);
        Buffer::free();
    }
}
