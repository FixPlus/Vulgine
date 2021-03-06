//
// Created by Бушев Дмитрий on 20.07.2021.
//

#ifndef TEST_EXE_VULGINEDESCRIPTORSET_H
#define TEST_EXE_VULGINEDESCRIPTORSET_H

#include "vulkan/VulkanDescriptable.h"
#include "vulkan/VulkanDescriptorPool.h"
#include "VulgineObject.h"
#include <vector>

namespace Vulgine{


    class DescriptorSet: public ObjectImpl{
        struct Set{
            uint32_t set = UINT32_MAX;
            std::vector<Descriptable*> descriptors;
        };
        std::vector<Set> sets;


    protected:
        void createImpl() override;
        void destroyImpl() override;
    public:
        DescriptorPool* pool = nullptr;



        DescriptorSet(): ObjectImpl(Type::DESCRIPTOR_SET, ObjectImpl::claimId()){}

        void bind(uint32_t set, VkCommandBuffer buffer, VkPipelineLayout layout, VkPipelineBindPoint bindPoint,uint32_t currentFrame);

        void addCombinedImageSampler(ImageRef const& image, VkShaderStageFlagBits stage, SamplerRef const& sampler);

        void addUniformBuffer(UniformBufferRef const& buffer, VkShaderStageFlagBits stage);

        void addInputAttachment(std::vector<VkImageView> const& views, VkShaderStageFlagBits stage);

        VkDescriptorSetLayout layout() const;

        bool empty() const { return sets.empty();};

        void clearDescriptors();

        void freeSets();

        ~DescriptorSet() override;
    };
}
#endif //TEST_EXE_VULGINEDESCRIPTORSET_H
