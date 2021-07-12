//
// Created by Бушев Дмитрий on 07.07.2021.
//

#ifndef TEST_EXE_VULKANALLOCATABLE_H
#define TEST_EXE_VULKANALLOCATABLE_H

#include "vma/vk_mem_alloc.h"


namespace Vulgine::Memory{

    struct Allocatable {

        VmaAllocation allocation;
        bool allocated = false;

        virtual ~Allocatable() = default;
    };


    struct Buffer : public Allocatable {
        VkBuffer buffer;

        void allocate(VkBufferCreateInfo bufferCI, VmaMemoryUsage memoryUsageFlags,
                      VmaAllocationCreateFlags allocFlags = 0,
                      VkMemoryPropertyFlags reqFlags = 0,
                      VkMemoryPropertyFlags prefFlags = 0);

        virtual void free();

        ~Buffer() override;
    };

    struct Image: public Allocatable{
        VkImage image;
        VkImageCreateInfo imageInfo;

        void allocate(VkImageCreateInfo imageCI, VmaMemoryUsage memoryUsageFlags,
                      VmaAllocationCreateFlags allocFlags = 0,
                      VkMemoryPropertyFlags reqFlags = 0,
                      VkMemoryPropertyFlags prefFlags = 0);

        void free();

        void transitImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        void copyFromBuffer(VkBuffer buffer, uint32_t width, uint32_t height, uint32_t depth = 1);

        virtual VkImageView createImageView() const;

        ~Image() override;
    };

    class StagingBuffer final: public Buffer{
        void* mapped = nullptr;
        uint32_t size = 0;
    public:
        void create(uint32_t size);

        void fill(const void* data);

        ~StagingBuffer() override;
    };

    /**
     *
     *
     *  @ImmutableBuffer - used for buffer object copied once to device memory and used consistently
     *  (e.g. static Vertex buffer)
     *
     *
     *
     */

    struct ImmutableBuffer: virtual public Buffer{
    protected:
        void create(void* pData, size_t size, VkBufferUsageFlagBits usage);
    public:
        void free() override;
    };

    /**
    *
    *
    *  @DynamicBuffer - used for buffer object frequently overriding by CPU and submitted to device memory
    *  (e.g. dynamic Vertex buffer)
    *
    *
    *
    */

    class DynamicBuffer: virtual public Buffer{
        void* mapped = nullptr;


        size_t dataSize = 0;
    protected:
        void create(size_t size, VkBufferUsageFlagBits usage);
    public:

        void push(void* data, size_t size = 0, size_t offset = 0);

        void free() override;

        ~DynamicBuffer() override;
    };

    struct VertexBuffer: virtual public Buffer{
        uint32_t binding = 0;
        void bind(VkCommandBuffer cmdBuffer);
    };

    struct IndexBuffer: virtual public Buffer{

        void bind(VkCommandBuffer cmdBuffer);

    };

    struct StaticVertexBuffer: public ImmutableBuffer, public VertexBuffer{

        void create(void* pData, size_t size);

    };

    struct DynamicVertexBuffer: public DynamicBuffer, public VertexBuffer{

        void create(size_t size);

    };

    struct StaticIndexBuffer: public IndexBuffer, public ImmutableBuffer{

        void create(uint32_t * pData, size_t size);


    };

    struct DynamicIndexBuffer: public IndexBuffer, public DynamicBuffer{

        void create(size_t size);

    };


}

#endif //TEST_EXE_VULKANALLOCATABLE_H
