//
// Created by Бушев Дмитрий on 05.07.2021.
//

#include "VulginePipeline.h"
#include "Vulgine.h"
#include "Utilities.h"
#include "vulkan/VulkanInitializers.hpp"
#include <vector>

void Vulgine::Pipeline::createImpl() {


        std::vector<VkDescriptorSetLayout> layouts;

        if(material->hasDescriptorSet)
            layouts.push_back(vlg_instance->perMaterialPool.getLayout(material->descriptorSet));
        if(!mesh->descriptors.empty())
            layouts.push_back(vlg_instance->perMeshPool.getLayout(mesh->descriptorSet));

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
                initializers::pipelineLayoutCreateInfo(
                layouts.empty() ? nullptr : layouts.data(),
                        layouts.size());

        // setting up arbitrary push constant range containing view Matrix of a camera

        VkPushConstantRange push_constant;
        //this push constant range starts at the beginning
        push_constant.offset = 0;
        //this push constant range takes up the size of a 4x4 matrix
        push_constant.size = sizeof(glm::mat4) + sizeof(glm::vec3);
        //this push constant range is accessible only in the vertex shader
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        pPipelineLayoutCreateInfo.pPushConstantRanges = &push_constant;
        pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;

        VK_CHECK_RESULT(vkCreatePipelineLayout(vlg_instance->device->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));


        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
        VkPipelineRasterizationStateCreateInfo rasterizationState = initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
        VkPipelineColorBlendAttachmentState blendAttachmentState = initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
        VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
        VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo(1, 1, 0);
        VkPipelineMultisampleStateCreateInfo multisampleState = initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
        std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamicState = initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables, 0);
        std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages {};
        VkGraphicsPipelineCreateInfo pipelineCI = initializers::pipelineCreateInfo(pipelineLayout, renderPass->renderPass, 0);
        pipelineCI.pInputAssemblyState = &inputAssemblyState;
        pipelineCI.pRasterizationState = &rasterizationState;
        pipelineCI.pColorBlendState = &colorBlendState;
        pipelineCI.pMultisampleState = &multisampleState;
        pipelineCI.pViewportState = &viewportState;
        pipelineCI.pDepthStencilState = &depthStencilState;
        pipelineCI.pDynamicState = &dynamicState;
        pipelineCI.stageCount = shaderStages.size();
        pipelineCI.pStages = shaderStages.data();
        pipelineCI.pVertexInputState = &mesh->vertexInputStateCI;

        // binding vertex shader

        auto const& vertexShaderName = mesh->vertexStageInfo.vertexShader;

        if(vlg_instance->vertexShaders.count(vertexShaderName) == 0){
            Utilities::ExitFatal(-1,"Mesh has unknown vertex shader");
        }
        auto& vertexShader = vlg_instance->vertexShaders[vertexShaderName];


        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertexShader.module;
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].pName = "main";

        // binding fragment shader

        const char* defaultFragmentShader;

        if(material->texture.colorMap){
            if(material->texture.normalMap){
                defaultFragmentShader = "frag_color_normal";
            }else{
                defaultFragmentShader = "frag_color";
            }
        }
        else{
            defaultFragmentShader = "frag_default";
        }


        if(vlg_instance->fragmentShaders.count(defaultFragmentShader) == 0){
            Utilities::ExitFatal(-1,"Error loading default fragment shader");
        }
        auto& fragmentShader = vlg_instance->fragmentShaders[defaultFragmentShader];


        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragmentShader.module;
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].pName = "main";


        VK_CHECK_RESULT(
                vkCreateGraphicsPipelines(vlg_instance->device->logicalDevice, vlg_instance->pipelineCache, 1, &pipelineCI, nullptr, &pipeline));



}

void Vulgine::Pipeline::destroyImpl() {
    vkDestroyPipelineLayout(vlg_instance->device->logicalDevice, pipelineLayout, nullptr);
    vkDestroyPipeline(vlg_instance->device->logicalDevice, pipeline, nullptr);

    pipeline = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;

}

Vulgine::Pipeline::~Pipeline() {
    if(isCreated()) {
        vkDestroyPipelineLayout(vlg_instance->device->logicalDevice, pipelineLayout, nullptr);
        vkDestroyPipeline(vlg_instance->device->logicalDevice, pipeline, nullptr);

        pipeline = VK_NULL_HANDLE;
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void Vulgine::Pipeline::bind(VkCommandBuffer cmdBuffer) const {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

bool Vulgine::PipelineKey::operator<(PipelineKey const& another) const {
    if(mesh < another.mesh)
        return true;
    if(mesh == another.mesh) {
        if(material < another.material)
            return true;
        if(material == another.material){
            if(scene < another.scene)
                return true;
            if(scene == another.scene){
                if(renderPass < another.renderPass)
                    return true;
            }
        }
    }

    return false;
}

