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

        int framebufferCount = vlg_instance->settings.framesInFlight;

        if(vertices.dynamic){

            for(int i = 0; i < framebufferCount; ++i) {
                auto* dynamicBuf = new Memory::DynamicVertexBuffer{};
                dynamicBuf->create(vertices.count * vertexStageInfo.vertexFormat.perVertexSize());
                dynamicBuf->binding = 0;
                perVertex.emplace_back(dynamicBuf, false);
            }

        }
        else{
            auto* staticBuf = new Memory::StaticVertexBuffer{};
            staticBuf->create(vertices.pData, vertices.count * vertexStageInfo.vertexFormat.perVertexSize());
            staticBuf->binding = 0;
            perVertex.emplace_back(staticBuf, false);
        }

        compileVertexInputState();

        // Vulgine allows rendering unindexed meshes

        if(!indices.empty())
            indexBuffer.create(indices.data(), indices.size());


        // we will use instance buffer if we actually have multiple instances

        if(instances.count > 0) {
            if (instances.dynamic) {

                for(int i = 0; i < framebufferCount; ++i) {
                    auto *dynamicBuf = new Memory::DynamicVertexBuffer{};
                    dynamicBuf->create(instances.count * vertexStageInfo.vertexFormat.perInstanceSize());
                    dynamicBuf->binding = 1;
                    perInstance.emplace_back(dynamicBuf, false);
                }

            } else {
                auto *staticBuf = new Memory::StaticVertexBuffer{};
                staticBuf->create(instances.pData, instances.count * vertexStageInfo.vertexFormat.perInstanceSize());
                staticBuf->binding = 1;
                perInstance.emplace_back(staticBuf, false);
            }

        }



        std::vector<std::vector<VkDescriptorSetLayoutBinding>> bindings;
        std::vector<std::vector<VkWriteDescriptorSet>> writes;
        bindings.resize(framebufferCount);
        writes.resize(framebufferCount);

        for (auto it = vertexStageInfo.descriptors.begin(), end = vertexStageInfo.descriptors.end();
             it != end; ++it) {
            switch (it->type) {
                case DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER: {

                    // for now assuming that image is static

                    VkDescriptorSetLayoutBinding binding;
                    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    binding.binding = it->binding;
                    binding.descriptorCount = 1;
                    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                    binding.pImmutableSamplers = nullptr;

                    auto *image = dynamic_cast<ImageImpl *>(it->image);
                    auto *combImageSampler = new CombinedImageSampler{&image->image};

                    combImageSampler->setupDescriptor();

                    descriptors.emplace_back(std::vector<Descriptable *>{combImageSampler});

                    for(int i = 0; i < framebufferCount; i++) {

                        bindings[i].push_back(binding);
                        writes[i].emplace_back(combImageSampler->write(it->binding));

                    }
                    break;
                }
                case DescriptorInfo::Type::UNIFORM_BUFFER: {
                    VkDescriptorSetLayoutBinding binding;
                    binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    binding.binding = it->binding;
                    binding.descriptorCount = 1;
                    binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                    binding.pImmutableSamplers = nullptr;
                    descriptors.emplace_back();
                    if(it->ubo->dynamic){
                        for(int i = 0; i < framebufferCount; i++){
                            auto* buffer = dynamic_cast<UniformBufferImpl*>(it->ubo);
                            auto* ubo = new DUniformBuffer{buffer->buffers.at(i)};
                            ubo->setupDescriptor();
                            descriptors.back().emplace_back(ubo);

                            bindings[i].push_back(binding);
                            writes[i].emplace_back(ubo->write(it->binding));

                        }
                    } else{
                        auto* buffer = dynamic_cast<UniformBufferImpl*>(it->ubo);
                        auto* ubo = new DUniformBuffer{buffer->buffers.at(0)};
                        ubo->setupDescriptor();
                        descriptors.back().emplace_back(ubo);

                        for(int i = 0; i < framebufferCount; i++) {

                            bindings[i].push_back(binding);
                            writes[i].emplace_back(ubo->write(it->binding));

                        }
                    }
                    break;
                }
                default:
                    assert(0 && "Invalid descriptor type");
            }
        }

        if (!writes.at(0).empty()) {
            for(int i = 0; i < framebufferCount; i++) {
                descriptorSets.push_back(vlg_instance->perMeshPool.allocateSet(bindings[i].data(), bindings[i].size()));
                vlg_instance->perMeshPool.updateDescriptor(descriptorSets.back(), writes[i]);
            }
        }


    }

    void MeshImpl::destroyImpl() {
        for(auto buf: perVertex)
            delete buf.first;
        for(auto buf: perInstance)
            delete buf.first;
        for(auto const& desc: descriptors) {
            for(auto perFrame: desc){
                perFrame->destroyDescriptor();
                delete perFrame;
            }
        }

        perVertex.clear();
        perInstance.clear();
        descriptors.clear();

        if(indexBuffer.allocated)
            indexBuffer.free();
    }

    void MeshImpl::draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass *pass) {
        int currentFrame = vlg_instance->currentFrame;
        int vertBufId = vertices.dynamic ? currentFrame : 0;
        int instanceBufId = instances.dynamic ? currentFrame : 0;

        auto& vertexBuf = perVertex.at(vertBufId);
        if(vertexBuf.second) {
            pushVertexBuffer(vertBufId);
            vertexBuf.second = false;
        }
        auto& instanceBuf = perInstance.at(instanceBufId);
        if(instanceBuf.second) {
            pushInstanceBuffer(instanceBufId);
            instanceBuf.second = false;
        }

        vertexBuf.first->bind(commandBuffer);

        bool hasMeshDescriptors = !descriptors.empty();

        if(!perInstance.empty())
            instanceBuf.first->bind(commandBuffer);
        uint32_t instCount = instances.count == 0 ? 1 : instances.count;

        if(indices.empty()) {
            auto* material = dynamic_cast<MaterialImpl *>(primitives[0].material);
            auto& boundPipeline = vlg_instance->pipelineMap.bind({this, dynamic_cast<MaterialImpl *>(primitives[0].material),
                                            dynamic_cast<SceneImpl *>(parent()), pass}, commandBuffer);
            if(hasMeshDescriptors)
                vlg_instance->perMeshPool.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundPipeline.pipelineLayout, 1,descriptorSets.at(currentFrame));
            if(material->hasDescriptorSet)
                vlg_instance->perMaterialPool.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundPipeline.pipelineLayout, 0,material->descriptorSet);
            vkCmdPushConstants(commandBuffer, boundPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(camera->matrices), &(camera->matrices));
            vkCmdDraw(commandBuffer, vertices.count, instCount, 0, 0);
        }else{
            indexBuffer.bind(commandBuffer);
            for (auto primitive: primitives) {
                auto* material = dynamic_cast<MaterialImpl *>(primitive.material);
                auto& boundPipeline = vlg_instance->pipelineMap.bind({this, material,
                                                dynamic_cast<SceneImpl *>(parent()), pass}, commandBuffer);
                if(hasMeshDescriptors)
                    vlg_instance->perMeshPool.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundPipeline.pipelineLayout, 1,descriptorSets.at(currentFrame));
                if(material->hasDescriptorSet)
                    vlg_instance->perMaterialPool.bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, boundPipeline.pipelineLayout, 0,material->descriptorSet);
                vkCmdPushConstants(commandBuffer, boundPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(camera->matrices), &(camera->matrices));
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, instCount, primitive.startIdx, 0, 0);
            }
        }

    }

    uint32_t MeshImpl::count_ = 0;
    MeshImpl::~MeshImpl() {
        count_--;
        for(auto buf: perVertex)
            delete buf.first;
        for(auto buf: perInstance)
            delete buf.first;
        for(auto const& desc: descriptors) {
            for(auto perFrame: desc){
                perFrame->destroyDescriptor();
                delete perFrame;
            }
        }
    }

    void MeshImpl::updateVertexBuffer() {
        if(vertices.dynamic){
            for(auto& buf: perVertex)
                buf.second = true;
        }
        else{
            vkQueueWaitIdle(vlg_instance->queue);
            perVertex.at(0).first->free();
            auto* statVert = dynamic_cast<Memory::StaticVertexBuffer*>(perVertex.at(0).first);
            statVert->create(vertices.pData, vertices.count * vertexStageInfo.vertexFormat.perVertexSize());
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
            for(auto& buf: perInstance)
                buf.second = true;
        }
        else{
            vkQueueWaitIdle(vlg_instance->queue);
            perInstance.at(0).first->free();
            auto* statVert = dynamic_cast<Memory::StaticVertexBuffer*>(perInstance.at(0).first);
            statVert->create(instances.pData, instances.count * vertexStageInfo.vertexFormat.perInstanceSize());
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

        auto& format = vertexStageInfo.vertexFormat;


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

    void MeshImpl::pushVertexBuffer(int id) {
        auto* dynVert = dynamic_cast<Memory::DynamicVertexBuffer*>(perVertex.at(id).first);
        if(cachedVertices.pData != vertices.pData || cachedVertices.count != vertices.count) {
            dynVert->free();
            dynVert->create(vertices.count * vertexStageInfo.vertexFormat.perVertexSize());
            cachedVertices.pData = vertices.pData;
            cachedVertices.count = vertices.count;
        } else{
            dynVert->push(vertices.pData);
            return;
        }
    }

    void MeshImpl::pushInstanceBuffer(int id) {
        auto* dynVert = dynamic_cast<Memory::DynamicVertexBuffer*>(perInstance.at(id).first);
        if(cachedInstances.pData != instances.pData || cachedInstances.count != instances.count) {
            dynVert->free();
            dynVert->create(instances.count * vertexStageInfo.vertexFormat.perInstanceSize());
            cachedInstances.pData = instances.pData;
            cachedInstances.count = instances.count;
        } else{
            dynVert->push(instances.pData);
            return;
        }
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

    void UniformBufferImpl::createImpl() {
        if(dynamic){
            int imageCount = vlg_instance->settings.framesInFlight;
            for(int i = 0; i < imageCount; i++) {
                auto *dynBuffer = new Memory::DynamicBuffer{};
                dynBuffer->create(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
                buffers.emplace_back(dynBuffer);
            }
        } else{
            auto* staticBuffer = new Memory::ImmutableBuffer{};
            staticBuffer->create(pData, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
            buffers.emplace_back(staticBuffer);
        }

        updated.resize(buffers.size(), false);
    }

    void UniformBufferImpl::destroyImpl() {
        for(auto* buf: buffers)
            delete buf;

        buffers.clear();
        updated.clear();
    }

    UniformBufferImpl::~UniformBufferImpl() {
        for(auto* buf: buffers)
            delete buf;
    }

    void UniformBufferImpl::update() {
        if(dynamic) {
            int updSize = updated.size();
            for (int i = 0; i < updSize; ++i)
                updated.at(i) = true;
        } else{
            assert(0 && "Update of static ubo is not implemented yet");
        }
    }

    void UniformBufferImpl::sync() {
        if(!dynamic)
            return;

        int curFrame = vlg_instance->currentFrame;

        if(updated.at(curFrame)){
             auto* buf = dynamic_cast<Memory::DynamicBuffer*>(buffers.at(curFrame));
             buf->push(pData);
            updated.at(curFrame) = false;
        }
    }

    UniformBufferImpl::UniformBufferImpl(uint32_t id) : ObjectImpl(id, Type::UBO) {

    }


 }
