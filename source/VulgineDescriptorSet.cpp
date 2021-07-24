//
// Created by Бушев Дмитрий on 20.07.2021.
//

#include "VulgineDescriptorSet.h"
#include "VulgineObjects.h"
#include "Vulgine.h"

void Vulgine::DescriptorSet::createImpl() {
    assert(pool && "Descriptor pool must be specified for descriptor set before create() invocation");

    for(auto& set: sets){
        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkWriteDescriptorSet> writes;

        bindings.reserve(set.descriptors.size());
        writes.reserve(set.descriptors.size());

        int i = 0;
        for(auto* descriptor: set.descriptors){
            VkWriteDescriptorSet write;
            VkDescriptorSetLayoutBinding binding{};
            binding.descriptorCount = 1;
            binding.binding = i;
            binding.stageFlags = descriptor->stage;
            binding.pImmutableSamplers = nullptr;

            switch(descriptor->descriptorType){
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
                case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: break;
                default: assert(0 && "Descriptor type isn't supported");
            }

            binding.descriptorType = descriptor->descriptorType;
            write = descriptor->write(i);

            bindings.push_back(binding);
            writes.push_back(write);

            i++;
        }

        set.set = pool->allocateSet(bindings.data(), bindings.size());
        pool->updateDescriptor(set.set, writes);
    }
}

void Vulgine::DescriptorSet::destroyImpl() {
    //assert(0 && "cannot deallocate descriptor Sets individually yet");
}

Vulgine::DescriptorSet::~DescriptorSet() {
    clearDescriptors();
}

void Vulgine::DescriptorSet::bind(uint32_t set, VkCommandBuffer buffer, VkPipelineLayout layout, VkPipelineBindPoint bindPoint,uint32_t currentFrame) {
    pool->bind(buffer, bindPoint, layout, set, sets.at(currentFrame).set);
}

void Vulgine::DescriptorSet::clearDescriptors() {
    for(auto& set: sets){
        for(auto* descriptor: set.descriptors) {
            descriptor->destroyDescriptor();
            delete descriptor;
        }
        set.descriptors.clear();
    }

}

void Vulgine::DescriptorSet::addCombinedImageSampler(Image *image, VkShaderStageFlagBits stage, Sampler* sampler) {
    if(sets.empty())
        sets.resize(GetImpl().swapChain.imageCount);

    auto* samplerImpl = dynamic_cast<SamplerImpl*>(sampler);

    if(auto* staticImage = dynamic_cast<StaticImageImpl*>(image)){

        for(auto& set: sets){
            auto* desc = set.descriptors.emplace_back(new CombinedImageSampler{&staticImage->image, samplerImpl->sampler});
            desc->stage = stage;
            desc->setupDescriptor();
        }

    } else if(auto* dynamicImage = dynamic_cast<DynamicImageImpl*>(image)){
        int i = 0;
        auto setSize = sets.size();
        for(auto& set: sets){
            auto* desc = set.descriptors.emplace_back(new CombinedImageSampler{&dynamicImage->images.at((i + setSize - 1) % setSize), samplerImpl->sampler});
            desc->stage = stage;
            desc->setupDescriptor();
            i++;
        }
    } else {
        assert(0 && "Cannot create a descriptor for this type of image yet");
    }

}

void Vulgine::DescriptorSet::addUniformBuffer(UniformBuffer *buffer, VkShaderStageFlagBits stage) {
    if(sets.empty())
        sets.resize(GetImpl().swapChain.imageCount);

    if(buffer->dynamic){
        auto& ubo = *dynamic_cast<UniformBufferImpl*>(buffer);
        int i = 0;
        for(auto& set: sets){
            auto* desc = set.descriptors.emplace_back(new DUniformBuffer{ubo.buffers.at(i)});
            desc->stage = stage;
            desc->setupDescriptor();
            i++;
        }

    } else {
        auto& ubo = *dynamic_cast<UniformBufferImpl*>(buffer);
        for(auto& set: sets){
            auto* desc = set.descriptors.emplace_back(new DUniformBuffer{ubo.buffers.at(0)});
            desc->stage = stage;
            desc->setupDescriptor();
        }
    }
}

VkDescriptorSetLayout Vulgine::DescriptorSet::layout() const {
    assert(isCreated() && "Cannot return layout if descriptor set is not created");
    return pool->getLayout(sets.back().set);
}

void Vulgine::DescriptorSet::addInputAttachment(const std::vector<VkImageView> &views, VkShaderStageFlagBits stage) {
    if(sets.empty())
        sets.resize(GetImpl().swapChain.imageCount);
    int i = 0;
    for(auto& set: sets){
        auto* desc = set.descriptors.emplace_back(new DInputAttachment{views.at(i)});
        desc->stage = stage;
        desc->setupDescriptor();
        i++;
    }
}

