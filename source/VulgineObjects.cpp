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

        SELF_CHECK_DEVICE_LIMITS()

        cachedInstances.pData = instances.pData;
        cachedInstances.count = instances.count;

        cachedVertices.pData = vertices.pData;
        cachedVertices.count = vertices.count;

        int framebufferCount = GetImpl().settings.framesInFlight;

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


        for (auto & descriptor : vertexStageInfo.descriptors) {
            switch (descriptor.type) {
                case DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER: {

                    set.addCombinedImageSampler(descriptor.image, VK_SHADER_STAGE_VERTEX_BIT, descriptor.sampler);

                    break;
                }
                case DescriptorInfo::Type::UNIFORM_BUFFER: {

                    set.addUniformBuffer(descriptor.ubo, VK_SHADER_STAGE_VERTEX_BIT);

                    break;
                }
                default:
                    assert(0 && "Invalid descriptor type");
            }
        }

        set.pool = &GetImpl().perMeshPool;

        if(!set.empty()){
            set.create();
        }


    }

    void MeshImpl::destroyImpl() {
        for(auto buf: perVertex)
            delete buf.first;
        for(auto buf: perInstance)
            delete buf.first;

        set.destroy();
        set.clearDescriptors();

        perVertex.clear();
        perInstance.clear();

        if(indexBuffer.allocated)
            indexBuffer.free();
    }

    void MeshImpl::draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass *pass, int currentFrame) {
        int vertBufId = vertices.dynamic ? GetImpl().currentFrame : 0;
        int instanceBufId = instances.dynamic ? GetImpl().currentFrame : 0;

        auto& vertexBuf = perVertex.at(vertBufId);
        if(vertexBuf.second) {
            pushVertexBuffer(vertBufId);
            vertexBuf.second = false;
        }

        vertexBuf.first->bind(commandBuffer);

        if(instances.count > 0) {
            auto &instanceBuf = perInstance.at(instanceBufId);
            if (instanceBuf.second) {
                pushInstanceBuffer(instanceBufId);
                instanceBuf.second = false;
            }
            if (!perInstance.empty())
                instanceBuf.first->bind(commandBuffer);
        }


        bool hasMeshDescriptors = set.isCreated();


        uint32_t instCount = instances.count == 0 ? 1 : instances.count;

        if(indices.empty()) {
            auto* material = dynamic_cast<MaterialImpl *>(primitives[0].material);
            auto& boundPipeline = GetImpl().pipelineMap.bind({this, dynamic_cast<MaterialImpl *>(primitives[0].material),
                                            dynamic_cast<SceneImpl *>(parent()), dynamic_cast<RenderPassImpl *>(pass)}, commandBuffer);

           // dynamic_cast<SceneImpl*>(parent())->set.bind(0, commandBuffer, boundPipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);

            if(hasMeshDescriptors)
                set.bind(1, commandBuffer, boundPipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);

            if(material->set.isCreated())
                material->set.bind(0, commandBuffer, boundPipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);

            vkCmdPushConstants(commandBuffer, boundPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(camera->matrices), &(camera->matrices));
            vkCmdDraw(commandBuffer, vertices.count, instCount, 0, 0);
        }else{
            indexBuffer.bind(commandBuffer);
            for (auto primitive: primitives) {
                auto* material = dynamic_cast<MaterialImpl *>(primitive.material);
                auto& boundPipeline = GetImpl().pipelineMap.bind({this, material,
                                                dynamic_cast<SceneImpl *>(parent()), dynamic_cast<RenderPassImpl *>(pass)}, commandBuffer);

               // dynamic_cast<SceneImpl*>(parent())->set.bind(0, commandBuffer, boundPipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);

                if(hasMeshDescriptors)
                    set.bind(1, commandBuffer, boundPipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);

                if(material->set.isCreated())
                    material->set.bind(0, commandBuffer, boundPipeline.pipelineLayout, VK_PIPELINE_BIND_POINT_GRAPHICS, currentFrame);

                vkCmdPushConstants(commandBuffer, boundPipeline.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(camera->matrices), &(camera->matrices));
                vkCmdDrawIndexed(commandBuffer, primitive.indexCount, instCount, primitive.startIdx, 0, 0);
            }
        }

    }

    MeshImpl::~MeshImpl() {
        if(isCreated()) {
            for (auto buf: perVertex)
                delete buf.first;
            for (auto buf: perInstance)
                delete buf.first;

            set.destroy();
        }
    }

    void MeshImpl::updateVertexBuffer() {
        if(vertices.dynamic){
            for(auto& buf: perVertex)
                buf.second = true;
        }
        else{
            vkQueueWaitIdle(GetImpl().queue);
            perVertex.at(0).first->free();
            auto* statVert = dynamic_cast<Memory::StaticVertexBuffer*>(perVertex.at(0).first);
            statVert->create(vertices.pData, vertices.count * vertexStageInfo.vertexFormat.perVertexSize());
        }

        GetImpl().cmdBuffersOutdated = true;
    }

    void MeshImpl::updateIndexBuffer() {
        indexBuffer.free();
        indexBuffer.create(indices.data(), indices.size());

        GetImpl().cmdBuffersOutdated = true;
    }

    void MeshImpl::updateInstanceBuffer() {
        if(instances.dynamic){
            for(auto& buf: perInstance)
                buf.second = true;
        }
        else{
            vkQueueWaitIdle(GetImpl().queue);
            perInstance.at(0).first->free();
            auto* statVert = dynamic_cast<Memory::StaticVertexBuffer*>(perInstance.at(0).first);
            statVert->create(instances.pData, instances.count * vertexStageInfo.vertexFormat.perInstanceSize());
        }

        GetImpl().cmdBuffersOutdated = true;

    }

    void MaterialImpl::createImpl() {

        set.pool = &GetImpl().perMaterialPool;

        if(!custom) {
            assert(!texture.normalMap ||
                   (texture.colorMap && texture.normalMap) && "Standalone normal maps aren't supported");
            assert(((texture.colorMap && texture.sampler) || !texture.colorMap) && "Cannot create material texture without sampler");
            if (texture.colorMap) {
                set.addCombinedImageSampler(texture.colorMap, VK_SHADER_STAGE_FRAGMENT_BIT, texture.sampler);

            }
            if (texture.normalMap) {
                set.addCombinedImageSampler(texture.normalMap, VK_SHADER_STAGE_FRAGMENT_BIT, texture.sampler);

            }

            if (texture.colorMap) {
                set.create();
            }
        } else{
            for(auto& descriptor: customMaterialInfo.descriptors){
                switch(descriptor.type){
                    case DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER:{

                        set.addCombinedImageSampler(descriptor.image, VK_SHADER_STAGE_FRAGMENT_BIT, descriptor.sampler);

                        break;
                    }
                    case DescriptorInfo::Type::UNIFORM_BUFFER:{
                        set.addUniformBuffer(descriptor.ubo, VK_SHADER_STAGE_FRAGMENT_BIT);
                        break;
                    }
                    default: assert(0 && "Invalid descriptor type");
                }
            }
            if(!customMaterialInfo.descriptors.empty())
                set.create();
        }

    }

    void MaterialImpl::destroyImpl() {
        set.destroy();
        set.clearDescriptors();
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

    }

    void LightImpl::createImpl() {

    }

    void LightImpl::destroyImpl() {

    }

    void LightImpl::update() {
        dynamic_cast<SceneImpl*>(parent())->updateLight(id());
    }

    ShaderModule::~ShaderModule() {
        vkDestroyShaderModule(GetImpl().device->logicalDevice, module, nullptr);
    }

    void ShaderModule::createImpl() {
        // actual creation is happening outside
    }

    void ShaderModule::destroyImpl() {
        vkDestroyShaderModule(GetImpl().device->logicalDevice, module, nullptr);
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

        GetImpl().cmdBuffersOutdated = true;
    }

    void UniformBufferImpl::createImpl() {
        if(dynamic){
            int imageCount = GetImpl().swapChain.imageCount;
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

        int curFrame = GetImpl().currentBuffer;

        if(updated.at(curFrame)){
             auto* buf = dynamic_cast<Memory::DynamicBuffer*>(buffers.at(curFrame));
             buf->push(pData);
            updated.at(curFrame) = false;
        }
    }

    UniformBufferImpl::UniformBufferImpl(uint32_t id) : ObjectImplNoMove(Type::UBO, id) {

    }


    void SamplerImpl::createImpl() {
        VkFilter filter;

        switch(filtering){
            case Sampler::Filtering::LINEAR: filter = VK_FILTER_LINEAR; break;
            case Sampler::Filtering::NONE: filter = VK_FILTER_NEAREST; break;
        }

        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = filter;
        samplerCreateInfo.minFilter = filter;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        // Max level-of-detail should match mip level count
        samplerCreateInfo.maxLod = 0.0f;
        // Only enable anisotropic filtering if enabled on the device
        samplerCreateInfo.maxAnisotropy = GetImpl().device->enabledFeatures.samplerAnisotropy ? GetImpl().device->properties.limits.maxSamplerAnisotropy : 1.0f;
        samplerCreateInfo.anisotropyEnable = GetImpl().device->enabledFeatures.samplerAnisotropy;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        VK_CHECK_RESULT(vkCreateSampler(GetImpl().device->logicalDevice, &samplerCreateInfo, nullptr, &sampler));

    }

    SamplerImpl::~SamplerImpl() {
        if(isCreated())
            vkDestroySampler(GetImpl().device->logicalDevice, sampler, nullptr);
    }

    void SamplerImpl::destroyImpl() {
        vkDestroySampler(GetImpl().device->logicalDevice, sampler, nullptr);
        sampler = VK_NULL_HANDLE;
    }
}
