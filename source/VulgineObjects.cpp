//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include <glm/glm.hpp>
#include "VulgineObjects.h"
#include "Utilities.h"
#include "Vulgine.h"

namespace Vulgine{

    void MeshImpl::createImpl() {

    }

    void MeshImpl::updateVertexData() {

    }

    void MeshImpl::updateInstanceData() {

    }

    void MeshImpl::destroyImpl() {

    }

    void MaterialImpl::createImpl() {

    }

    void MaterialImpl::destroyImpl() {

    }

    void MaterialImpl::compileVertexInputState() {

        uint32_t perVertexSize = 0, perInstanceSize = 0;

        attributesDesc.clear();

        auto& format = vertexFormat;

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
    }

    MaterialImpl::~MaterialImpl() {

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
}
