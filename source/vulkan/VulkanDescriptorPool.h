//
// Created by Бушев Дмитрий on 10.07.2021.
//

#ifndef TEST_EXE_VULKANDESCRIPTORPOOL_H
#define TEST_EXE_VULKANDESCRIPTORPOOL_H

#include <vulkan/vulkan.h>
#include <map>
#include <vector>
#include <stack>

namespace Vulgine{

    class DescriptorPool{
        std::vector<VkDescriptorPool> pools{};
        uint32_t setCount = 0;

        std::map<VkDescriptorType, uint32_t> descriptorsStored{};

        std::map<uint32_t, VkDescriptorSet> sets{};
        std::map<VkDescriptorSet, VkDescriptorPool> allocatedIn{};
        std::map<VkDescriptorSet, VkDescriptorSetLayout> layouts{};

        std::stack<uint32_t> freeIds;

    public:
        std::map<VkDescriptorType, uint32_t> descriptorsCapacity{};
        uint32_t maxSets = 0;

        uint32_t allocateSet(const VkDescriptorSetLayoutBinding*, uint32_t bindingCount);

        void bind(VkCommandBuffer cmdBuffer, VkPipelineBindPoint pipelineType, VkPipelineLayout layout, uint32_t set, uint32_t id) const;

        void updateDescriptor(uint32_t id, std::vector<VkWriteDescriptorSet> writes);

        VkDescriptorSetLayout getLayout(uint32_t id);

        void freeSet(uint32_t id);

        void clear();

        void reset();
    };
}
#endif //TEST_EXE_VULKANDESCRIPTORPOOL_H
