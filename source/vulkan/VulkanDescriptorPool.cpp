//
// Created by Бушев Дмитрий on 10.07.2021.
//

#include <cassert>
#include "VulkanDescriptorPool.h"
#include "Utilities.h"
#include <algorithm>
#include "vulkan/VulkanInitializers.hpp"
#include "Vulgine.h"

uint32_t Vulgine::DescriptorPool::allocateSet(const VkDescriptorSetLayoutBinding* bindings, uint32_t bindingCount) {

    for(uint32_t i = 0; i < bindingCount; ++i) {
        auto type = bindings[i].descriptorType;
        assert(descriptorsCapacity.count(type) && "Must allocate descriptors only of appropriate types");
        if(descriptorsStored.count(type))
            descriptorsStored.at(type)++;
        else
            descriptorsStored.emplace(type, 1);
    }

    setCount++;

    if(pools.empty()){
        std::vector<VkDescriptorPoolSize> poolSizes;
        for(auto cap: descriptorsCapacity)
            poolSizes.push_back({cap.first, cap.second});
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = initializers::descriptorPoolCreateInfo(poolSizes, maxSets);

        if(returnable)
            descriptorPoolCreateInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        auto& newPool = pools.emplace_back();
        VK_CHECK_RESULT( vkCreateDescriptorPool(GetImpl().device->logicalDevice, &descriptorPoolCreateInfo, nullptr, &newPool))
    }

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = initializers::descriptorSetLayoutCreateInfo(bindings, bindingCount);
    VkDescriptorSetLayout layout;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(GetImpl().device->logicalDevice, &layoutCreateInfo, nullptr, &layout))

    VkDescriptorSetAllocateInfo allocateInfo = initializers::descriptorSetAllocateInfo(pools.back(), &layout, 1);

    VkDescriptorSet descriptorSet;

    bool created = false;
    for(int i = 0; i < 2; i++){
        allocateInfo.descriptorPool = pools.back();
        auto result = vkAllocateDescriptorSets(GetImpl().device->logicalDevice, &allocateInfo, &descriptorSet);
        if(result == VK_SUCCESS){
            created = true;
            break;
        } else {
            std::vector<VkDescriptorPoolSize> poolSizes;
            for(auto cap: descriptorsCapacity)
                poolSizes.push_back({cap.first, cap.second});
            VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = initializers::descriptorPoolCreateInfo(poolSizes, maxSets);

            if(returnable)
                descriptorPoolCreateInfo.flags |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

            auto& newPool = pools.emplace_back();
            VK_CHECK_RESULT( vkCreateDescriptorPool(GetImpl().device->logicalDevice, &descriptorPoolCreateInfo, nullptr, &newPool))
        }
    }

    if(!created)
        Utilities::ExitFatal(-1, "Failed to allocate set");

    uint32_t id;
    if(freeIds.empty())
       id = sets.size();
    else {
        id = freeIds.top();
        freeIds.pop();
    }

    sets.emplace(id, descriptorSet);
    allocatedIn.emplace(descriptorSet, pools.back());
    layouts.emplace(descriptorSet, layout);

    return id;
}

void Vulgine::DescriptorPool::freeSet(uint32_t id) {
    assert(returnable && "Individual descriptor allocation return is not implemented yet");

    auto descriptor = sets.at(id);
    VK_CHECK_RESULT(vkFreeDescriptorSets(GetImpl().device->logicalDevice, allocatedIn.at(sets.at(id)), 1, &descriptor))

    allocatedIn.erase(descriptor);

    vkDestroyDescriptorSetLayout(GetImpl().device->logicalDevice, layouts.at(descriptor), nullptr);
    layouts.erase(descriptor);
    sets.erase(id);

    freeIds.push(id);
    setCount--;

}

void Vulgine::DescriptorPool::clear() {
    reset();
    vkDestroyDescriptorPool(GetImpl().device->logicalDevice, pools.at(0),nullptr);
    pools.clear();
}

void Vulgine::DescriptorPool::reset() {
    for(auto set: sets)
        vkDestroyDescriptorSetLayout(GetImpl().device->logicalDevice, layouts.at(set.second), nullptr);
    sets.clear();
    layouts.clear();
    descriptorsStored.clear();
    allocatedIn.clear();
    while(!freeIds.empty())
        freeIds.pop();

    setCount = 0;
    for(auto pool: pools){
        VK_CHECK_RESULT(vkResetDescriptorPool(GetImpl().device->logicalDevice, pool, 0))
    }

    for(int i = 1; i < pools.size(); i++)
        vkDestroyDescriptorPool(GetImpl().device->logicalDevice, pools.at(i),nullptr);
    pools.resize(1);
}

void Vulgine::DescriptorPool::bind(VkCommandBuffer cmdBuffer, VkPipelineBindPoint pipelineType, VkPipelineLayout layout, uint32_t set, uint32_t id) const{
    vkCmdBindDescriptorSets(cmdBuffer, pipelineType, layout, set, 1, &sets.at(id), 0, nullptr);
}

void Vulgine::DescriptorPool::updateDescriptor(uint32_t id, std::vector<VkWriteDescriptorSet> writes) {
    for(auto& write: writes)
        write.dstSet = sets.at(id);

    vkUpdateDescriptorSets(GetImpl().device->logicalDevice, writes.size(), writes.data(), 0, nullptr);
}

VkDescriptorSetLayout Vulgine::DescriptorPool::getLayout(uint32_t id) {
    return layouts.at(sets.at(id));
}
