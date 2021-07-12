//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include <glm/glm.hpp>
#include "VulgineObjects.h"
#include "Vulgine.h"
#include "VulgineImage.h"
#include "vulkan/VulkanInitializers.hpp"

namespace Vulgine{


    uint32_t VertexFormat::perVertexSize() const {
        uint32_t ret = 0;
        for(auto attr: perVertexAttributes){
            switch (attr) {
                case AttributeFormat::R32SF: ret += sizeof(float); break;
                case AttributeFormat::RG32SF: ret += sizeof(glm::vec2); break;
                case AttributeFormat::RGB32SF: ret += sizeof(glm::vec3); break;
                case AttributeFormat::RGBA32SF: ret += sizeof (glm::vec4); break;
                case AttributeFormat::MAT4F: ret+= sizeof (glm::mat4); break;
            }
        }
        return ret;
    }

    uint32_t VertexFormat::perInstanceSize() const {
        uint32_t ret = 0;
        for(auto attr: perInstanceAttributes){
            switch (attr) {
                case AttributeFormat::R32SF: ret += sizeof(float); break;
                case AttributeFormat::RG32SF: ret += sizeof(glm::vec2); break;
                case AttributeFormat::RGB32SF: ret += sizeof(glm::vec3); break;
                case AttributeFormat::RGBA32SF: ret += sizeof (glm::vec4); break;
                case AttributeFormat::MAT4F: ret+= sizeof (glm::mat4); break;
            }
        }
        return ret;
    }

    void MeshImpl::createImpl() {

        assert(vertices.count && "Can't create Mesh with 0 vertices");
        assert(!primitives.empty() && "Can't create Mesh with 0 primitives");
        assert(!indices.empty() || primitives.size() == 1 && "Unindexed mesh cant have more than one primitive");

        cachedInstances.pData = instances.pData;
        cachedInstances.count = instances.count;

        cachedVertices.pData = vertices.pData;
        cachedVertices.count = vertices.count;

        if(vertices.dynamic){
            auto* dynamicBuf = new Memory::DynamicVertexBuffer{};
            dynamicBuf->create(vertices.count * vertexFormat.perVertexSize());
            perVertex = dynamicBuf;
        }
        else{
            auto* staticBuf = new Memory::StaticVertexBuffer{};
            staticBuf->create(vertices.pData, vertices.count * vertexFormat.perVertexSize());
            perVertex = staticBuf;
        }

        compileVertexInputState();

        // Vulgine allows rendering unindexed meshes

        if(!indices.empty())
            indexBuffer.create(indices.data(), indices.size());

        perVertex->binding = 0;

        // we will use instance buffer if we actually have multiple instances

        if(instances.count > 0) {
            if (instances.dynamic) {
                auto *dynamicBuf = new Memory::DynamicVertexBuffer{};
                dynamicBuf->create(instances.count * vertexFormat.perInstanceSize());
                perInstance = dynamicBuf;
            } else {
                auto *staticBuf = new Memory::StaticVertexBuffer{};
                staticBuf->create(instances.pData, instances.count * vertexFormat.perInstanceSize());
                perInstance = staticBuf;
            }

            perInstance->binding = 1;
        }
    }

    void MeshImpl::destroyImpl() {
        delete perVertex;
        delete perInstance;
        perVertex = nullptr;
        perInstance = nullptr;
        if(indexBuffer.allocated)
            indexBuffer.free();
    }

    void MeshImpl::draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass *pass) {
        perVertex->bind(commandBuffer);

        if(perInstance)
            perInstance->bind(commandBuffer);
        uint32_t instCount = instances.count == 0 ? 1 : instances.count;

        if(indices.empty()) {
            auto* material = dynamic_cast<MaterialImpl *>(primitives[0].material);
            auto& boundPipeline = vlg_instance->pipelineMap.bind({vertexInputStateCI, dynamic_cast<MaterialImpl *>(primitives[0].material),
                                            dynamic_cast<SceneImpl *>(parent()), pass}, commandBuffer);
            if(material->hasDescriptorSet)
                vlg_instance->perMaterialPool.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundPipeline.pipelineLayout, 0,material->descriptorSet);
            vkCmdPushConstants(commandBuffer, boundPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(camera->matrices), &(camera->matrices));
            vkCmdDraw(commandBuffer, vertices.count, instCount, 0, 0);
        }else{
            indexBuffer.bind(commandBuffer);
            for (auto primitive: primitives) {
                auto* material = dynamic_cast<MaterialImpl *>(primitive.material);
                auto& boundPipeline = vlg_instance->pipelineMap.bind({vertexInputStateCI, material,
                                                dynamic_cast<SceneImpl *>(parent()), pass}, commandBuffer);
                if(material->hasDescriptorSet)
                    vlg_instance->perMaterialPool.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundPipeline.pipelineLayout, 0,material->descriptorSet);
                vkCmdPushConstants(commandBuffer, boundPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(camera->matrices), &(camera->matrices));
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, instCount, primitive.startIdx, 0, 0);
            }
        }

    }

    MeshImpl::~MeshImpl() {
        logger("Mesh destroyed");
        delete perVertex;
        delete perInstance;
    }

    void MeshImpl::updateVertexBuffer() {
        if(vertices.dynamic){
            auto* dynVert = dynamic_cast<Memory::DynamicVertexBuffer*>(perVertex);
            if(cachedVertices.pData != vertices.pData || cachedVertices.count != vertices.count) {
                dynVert->free();
                dynVert->create(vertices.count * vertexFormat.perVertexSize());
                cachedVertices.pData = vertices.pData;
                cachedVertices.count = vertices.count;
            } else{
                dynVert->push(vertices.pData);
                return;
            }
        }
        else{
            vkQueueWaitIdle(vlg_instance->queue);
            perVertex->free();
            auto* statVert = dynamic_cast<Memory::StaticVertexBuffer*>(perVertex);
            statVert->create(vertices.pData, vertices.count * vertexFormat.perVertexSize());
        }

        vlg_instance->cmdBuffersOutdated = true;
    }

    void MeshImpl::updateIndexBuffer() {
        indexBuffer.free();
        indexBuffer.create(indices.data(), indices.size());

        vlg_instance->cmdBuffersOutdated = true;
    }

    void MeshImpl::updateInstanceBuffer() {
        if(instances.dynamic){
            auto* dynVert = dynamic_cast<Memory::DynamicVertexBuffer*>(perInstance);
            if(cachedInstances.pData != instances.pData || cachedInstances.count != instances.count) {
                dynVert->free();
                dynVert->create(instances.count * vertexFormat.perInstanceSize());
                cachedInstances.pData = instances.pData;
                cachedInstances.count = instances.count;
            } else{
                dynVert->push(instances.pData);
                return;
            }
        }
        else{
            vkQueueWaitIdle(vlg_instance->queue);
            perInstance->free();
            auto* statVert = dynamic_cast<Memory::StaticVertexBuffer*>(perInstance);
            statVert->create(instances.pData, instances.count * vertexFormat.perInstanceSize());
        }

        vlg_instance->cmdBuffersOutdated = true;

    }

    void MaterialImpl::createImpl() {
        assert(!texture.normalMap || (texture.colorMap && texture.normalMap) && "Standalone normal maps aren't supported");

        std::vector<VkDescriptorSetLayoutBinding> bindings;
        std::vector<VkWriteDescriptorSet> writes;

        if(texture.colorMap){

            VkDescriptorSetLayoutBinding binding;
            binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.binding = 0;
            binding.descriptorCount = 1;
            binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
            bindings.push_back(binding);

            auto& colorImage = dynamic_cast<ImageImpl*>(texture.colorMap)->image;
            colorMapSampled.descriptor.imageView = colorImage.createImageView();
            colorMapSampled.sampler.create();
            colorMapSampled.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            colorMapSampled.descriptor.sampler = colorMapSampled.sampler.sampler;

            VkWriteDescriptorSet write = initializers::writeDescriptorSet(nullptr, binding.descriptorType, 0, &colorMapSampled.descriptor);
            writes.push_back(write);
        }
        if(texture.normalMap){

            VkDescriptorSetLayoutBinding binding;
            binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            binding.binding = 1;
            binding.descriptorCount = 1;
            binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;
            bindings.push_back(binding);

            auto& colorImage = dynamic_cast<ImageImpl*>(texture.colorMap)->image;
            normalMapSampled.descriptor.imageView = colorImage.createImageView();
            normalMapSampled.sampler.create();
            normalMapSampled.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            normalMapSampled.descriptor.sampler = normalMapSampled.sampler.sampler;
            VkWriteDescriptorSet write = initializers::writeDescriptorSet(nullptr, binding.descriptorType, 1, &colorMapSampled.descriptor);
            writes.push_back(write);
        }

        if(texture.colorMap) {
            descriptorSet = vlg_instance->perMaterialPool.allocateSet(bindings.data(), bindings.size());
            vlg_instance->perMaterialPool.updateDescriptor(descriptorSet, writes);
            hasDescriptorSet = true;
        }


    }

    void MaterialImpl::destroyImpl() {
        if(texture.colorMap){
            vkDestroyImageView(vlg_instance->device->logicalDevice, colorMapSampled.descriptor.imageView, nullptr);
        }

        if(texture.normalMap){
            vkDestroyImageView(vlg_instance->device->logicalDevice, normalMapSampled.descriptor.imageView, nullptr);
        }
    }

    void MeshImpl::compileVertexInputState() {

        uint32_t perVertexSize = 0, perInstanceSize = 0;

        attributesDesc.clear();

        auto& format = vertexFormat;

        int adjustment = 0;

        for(auto attr: format.perVertexAttributes){
            switch(attr){
                case AttributeFormat::R32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             0,
                             VK_FORMAT_R32_SFLOAT,
                             perVertexSize}));
                    perVertexSize += sizeof(float);
                    break;
                case AttributeFormat::RG32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             0,
                             VK_FORMAT_R32G32_SFLOAT,
                             perVertexSize}));
                    perVertexSize += sizeof(glm::vec2);
                    break;
                case AttributeFormat::RGB32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             0,
                             VK_FORMAT_R32G32B32_SFLOAT,
                             perVertexSize}));
                    perVertexSize += sizeof(glm::vec3);
                    break;
                case AttributeFormat::RGBA32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             0,
                             VK_FORMAT_R32G32B32A32_SFLOAT,
                             perVertexSize}));
                    perVertexSize += sizeof(glm::vec4);
                    break;
                case AttributeFormat::MAT4F:
                    for(int i = 0; i < 4; ++i) {
                        attributesDesc.push_back(VkVertexInputAttributeDescription(
                                {static_cast<uint32_t>(attributesDesc.size()),
                                 0,
                                 VK_FORMAT_R32G32B32A32_SFLOAT,
                                 perVertexSize}));
                        perVertexSize += sizeof(glm::vec4);
                    }
                    break;
            }
        }


        for(auto attr: format.perInstanceAttributes){
            switch(attr){
                case AttributeFormat::R32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             1,
                             VK_FORMAT_R32_SFLOAT,
                             perInstanceSize}));
                    perInstanceSize += sizeof(float);
                    break;
                case AttributeFormat::RG32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             1,
                             VK_FORMAT_R32G32_SFLOAT,
                             perInstanceSize}));
                    perInstanceSize += sizeof(glm::vec2);
                    break;
                case AttributeFormat::RGB32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             1,
                             VK_FORMAT_R32G32B32_SFLOAT,
                             perInstanceSize}));
                    perInstanceSize += sizeof(glm::vec3);
                    break;
                case AttributeFormat::RGBA32SF:
                    attributesDesc.push_back(VkVertexInputAttributeDescription(
                            {static_cast<uint32_t>(attributesDesc.size()),
                             1,
                             VK_FORMAT_R32G32B32A32_SFLOAT,
                             perInstanceSize}));
                    perInstanceSize += sizeof(glm::vec4);
                    break;
                case AttributeFormat::MAT4F:
                    for(int i = 0; i < 4; ++i) {
                        attributesDesc.push_back(VkVertexInputAttributeDescription(
                                {static_cast<uint32_t>(attributesDesc.size()),
                                 1,
                                 VK_FORMAT_R32G32B32A32_SFLOAT,
                                 perInstanceSize}));
                        perInstanceSize += sizeof(glm::vec4);
                    }
                    break;
            }
        }

        VkPipelineVertexInputStateCreateInfo ret{};

        vertexAttrBindingInfo[0] = VkVertexInputBindingDescription({ 0, perVertexSize, VK_VERTEX_INPUT_RATE_VERTEX });
        vertexAttrBindingInfo[1] = VkVertexInputBindingDescription({ 1, perInstanceSize, VK_VERTEX_INPUT_RATE_INSTANCE });

        ret.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        ret.vertexAttributeDescriptionCount = attributesDesc.size();

        if(attributesDesc.empty()) {
            ret.pVertexAttributeDescriptions = nullptr;
        } else {
            ret.pVertexAttributeDescriptions = attributesDesc.data();
        }

        if(perVertexSize == 0){
            if(perInstanceSize == 0){
                ret.pVertexBindingDescriptions = nullptr;
                ret.vertexBindingDescriptionCount = 0;
            } else{
                ret.pVertexBindingDescriptions = vertexAttrBindingInfo + 1;
                ret.vertexBindingDescriptionCount = 1;

            }
        } else {
            ret.pVertexBindingDescriptions = vertexAttrBindingInfo;
            ret.vertexBindingDescriptionCount = (perInstanceSize == 0) ? 1 : 2;
        }

        vertexInputStateCI = ret;
    }

    MaterialImpl::~MaterialImpl() {
        if(isCreated()){
            if(texture.colorMap){
                vkDestroyImageView(vlg_instance->device->logicalDevice, colorMapSampled.descriptor.imageView, nullptr);
                colorMapSampled.sampler.destroy();

            }

            if(texture.normalMap){
                vkDestroyImageView(vlg_instance->device->logicalDevice, normalMapSampled.descriptor.imageView, nullptr);
                colorMapSampled.sampler.destroy();
            }
        }

    }

    void LightImpl::createImpl() {

    }

    void LightImpl::destroyImpl() {

    }

    ShaderModule::~ShaderModule() {
        vkDestroyShaderModule(vlg_instance->device->logicalDevice, module, nullptr);
    }

    void ShaderModule::createImpl() {
        // actual creation is happening outside
    }

    void ShaderModule::destroyImpl() {
        vkDestroyShaderModule(vlg_instance->device->logicalDevice, module, nullptr);
        module = VK_NULL_HANDLE;
    }

    void CameraImpl::createImpl() {

    }

    void CameraImpl::destroyImpl() {

    }

    void CameraImpl::update() {
        glm::mat4 rotM = glm::mat4(1.0f);
        glm::mat4 transM;

        rotM = glm::rotate(rotM, glm::radians(-rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(-rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotM = glm::rotate(rotM, glm::radians(-rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 translation = -position;
        //translation.y = translation.y * -1.0f;

        transM = glm::translate(glm::mat4(1.0f), translation);

        matrices.viewMatrix = projection * rotM * transM;
        matrices.position = position;

        vlg_instance->cmdBuffersOutdated = true;
    }
}
