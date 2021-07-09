//
// Created by Бушев Дмитрий on 07.07.2021.
//

#include "VulkanAllocatable.h"
#include "Vulgine.h"
#include "VulkanInitializers.hpp"



void Vulgine::Memory::Buffer::allocate(VkBufferCreateInfo bufferCI, VmaMemoryUsage memoryUsageFlags, VmaAllocationCreateFlags allocFlags, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags prefFlags) {
    if(allocated){
        vmaDestroyBuffer(vlg_instance->allocator, buffer, allocation);
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageFlags;
    allocInfo.preferredFlags = prefFlags;
    allocInfo.requiredFlags = reqFlags;
    allocInfo.flags = allocFlags;


    vmaCreateBuffer(vlg_instance->allocator, &bufferCI, &allocInfo, &buffer, &allocation, nullptr);

    allocated = true;
}

Vulgine::Memory::Buffer::~Buffer() {
    if(allocated){
        vmaDestroyBuffer(vlg_instance->allocator, buffer, allocation);
    }
}

void Vulgine::Memory::Buffer::free() {
    if(allocated){
        vmaDestroyBuffer(vlg_instance->allocator, buffer, allocation);
    }
    allocated = false;

    buffer = VK_NULL_HANDLE;
}

void Vulgine::Memory::Image::allocate(VkImageCreateInfo imageCI, VmaMemoryUsage memoryUsageFlags, VmaAllocationCreateFlags allocFlags, VkMemoryPropertyFlags reqFlags, VkMemoryPropertyFlags prefFlags) {
    if(allocated){
        vmaDestroyImage(vlg_instance->allocator, image, allocation);
    }

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = memoryUsageFlags;
    allocInfo.preferredFlags = prefFlags;
    allocInfo.requiredFlags = reqFlags;
    allocInfo.flags = allocFlags;

    vmaCreateImage(vlg_instance->allocator, &imageCI, &allocInfo, &image, &allocation, nullptr);

    allocated = true;

}

void Vulgine::Memory::Image::free() {
    if(allocated){
        vmaDestroyImage(vlg_instance->allocator, image, allocation);
    }

    allocated = false;

    image = VK_NULL_HANDLE;
}

Vulgine::Memory::Image::~Image() {
    if(allocated){
        vmaDestroyImage(vlg_instance->allocator, image, allocation);
    }
}

void Vulgine::Memory::ImmutableBuffer::create(void *pData, size_t size, VkBufferUsageFlagBits usage) {

    assert(pData && size && "Invalid data description");

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

    vmaMapMemory(vlg_instance->allocator, stagingBuffer.allocation, &mappedData);

    memcpy(mappedData, pData, size);

    vmaUnmapMemory(vlg_instance->allocator, stagingBuffer.allocation);


    // Copy from staging buffers

    VkCommandBuffer copyCmd = vlg_instance->device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

    VkBufferCopy copyRegion = {};

    copyRegion.size = size;
    vkCmdCopyBuffer(copyCmd, stagingBuffer.buffer, buffer, 1, &copyRegion);

    vlg_instance->device->flushCommandBuffer(copyCmd, vlg_instance->transferQueue, true);

    vkQueueWaitIdle(vlg_instance->transferQueue);



}

void Vulgine::Memory::VertexBuffer::bind(VkCommandBuffer cmdBuffer) {

    assert(allocated && "Trying to bind unallocated vertex buffer");

    const VkDeviceSize offset = 0;

    vkCmdBindVertexBuffers(cmdBuffer, binding, 1, &buffer, &offset);
}


void Vulgine::Memory::DynamicBuffer::create(void *pData, size_t size, VkBufferUsageFlagBits usage) {

    assert(pData && size && "Invalid data description");

    if(allocated) {
        vmaUnmapMemory(vlg_instance->allocator, allocation);
        free();
    }

    data = pData;
    dataSize = size;

    VkBufferCreateInfo bufferCI = initializers::bufferCreateInfo(usage, size);
    bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    allocate(bufferCI, VMA_MEMORY_USAGE_CPU_TO_GPU);

    vmaMapMemory(vlg_instance->allocator, allocation, &mapped);
}

Vulgine::Memory::DynamicBuffer::~DynamicBuffer() {
    if(allocated)
        vmaUnmapMemory(vlg_instance->allocator, allocation);
}

void Vulgine::Memory::DynamicBuffer::push() {
    assert(allocated && "Memory not allocated for this buffer yet");
    memcpy(mapped, data, dataSize);
}

void Vulgine::Memory::DynamicBuffer::free() {
    vmaUnmapMemory(vlg_instance->allocator, allocation);
    Buffer::free();
}

void Vulgine::Memory::StaticVertexBuffer::create(void *pData, size_t size) {
    ImmutableBuffer::create(pData, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void Vulgine::Memory::DynamicVertexBuffer::create(void *pData, size_t size) {
    DynamicBuffer::create(pData, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

void Vulgine::Memory::IndexBuffer::create(uint32_t *pData, size_t size) {
    ImmutableBuffer::create((void*)pData, size * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void Vulgine::Memory::IndexBuffer::free() {
    Buffer::free();
}

void Vulgine::Memory::IndexBuffer::bind(VkCommandBuffer cmdBuffer) {
    vkCmdBindIndexBuffer(cmdBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
}
