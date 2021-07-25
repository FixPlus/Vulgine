//
// Created by Бушев Дмитрий on 05.07.2021.
//

#include "VulginePipeline.h"
#include "Vulgine.h"
#include "vulkan/VulkanInitializers.hpp"
#include <vector>

namespace {
    VkPipelineVertexInputStateCreateInfo* emptyVertexState(){
        static bool created = false;
        static VkPipelineVertexInputStateCreateInfo state{};

        if(!created){
            created = true;
            state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            state.flags = 0;
            state.vertexAttributeDescriptionCount = 0;
            state.vertexBindingDescriptionCount = 0;
        }

        return &state;
    }
}
void Vulgine::GeneralPipeline::createImpl() {


        std::vector<VkDescriptorSetLayout> layouts;

        if(material->set.isCreated())
            layouts.push_back(material->set.layout());
        if(geometry && geometry->layout != VK_NULL_HANDLE)
            layouts.push_back(geometry->layout);

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

        VK_CHECK_RESULT(vkCreatePipelineLayout(GetImpl().device->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));


        VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
        VkPipelineRasterizationStateCreateInfo rasterizationState = initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
        VkPipelineColorBlendAttachmentState blendAttachmentState[10];
        auto colorAttachmentsCount = geometry == nullptr ? 1 : renderPass->frameBuffer.colorAttachmentCount();
        for(int i = 0; i < colorAttachmentsCount; i++)
            blendAttachmentState[i] = initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);

        VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::pipelineColorBlendStateCreateInfo(colorAttachmentsCount, blendAttachmentState);
        VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
        VkPipelineViewportStateCreateInfo viewportState = initializers::pipelineViewportStateCreateInfo(1, 1, 0);
        VkPipelineMultisampleStateCreateInfo multisampleState = initializers::pipelineMultisampleStateCreateInfo(renderPass == GetImpl().onscreenRenderPass.get() && !renderPass->deferredEnabled ? GetImpl().settings.msaa : VK_SAMPLE_COUNT_1_BIT, 0);
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
        pipelineCI.pVertexInputState = geometry ? &geometry->vertexInputStateCI :  emptyVertexState();

        pipelineCI.subpass = 0;
        if(geometry == nullptr){
            pipelineCI.subpass = renderPass->deferredEnabled ? 2 : 0;
            rasterizationState.cullMode = VK_CULL_MODE_NONE;
            depthStencilState.depthWriteEnable = VK_FALSE;
        }

        // binding vertex shader

        auto const& vertexShaderName = geometry ? geometry->vertexShader : "vert_background";

        if(GetImpl().vertexShaders.count(vertexShaderName) == 0){
            Utilities::ExitFatal(-1,"Mesh has unknown vertex shader");
        }
        auto& vertexShader = GetImpl().vertexShaders[vertexShaderName];


        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertexShader.module;
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].pName = "main";

        // binding fragment shader

        const char* defaultFragmentShader;

        if(material->custom){
            defaultFragmentShader = material->customMaterialInfo.fragmentShader.c_str();
        } else {
            if (material->texture.colorMap) {
                if (material->texture.normalMap) {
                    defaultFragmentShader = "frag_color_normal";
                } else {
                    if(renderPass->deferredEnabled)
                        defaultFragmentShader = "frag_gbuffer_textured";
                    else
                        defaultFragmentShader = "frag_color";
                }
            } else {
                defaultFragmentShader = "frag_default";
            }
        }

        if(GetImpl().fragmentShaders.count(defaultFragmentShader) == 0){
            Utilities::ExitFatal(-1,"Error loading default fragment shader");
        }
        auto& fragmentShader = GetImpl().fragmentShaders[defaultFragmentShader];


        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragmentShader.module;
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].pName = "main";


        VK_CHECK_RESULT(
                vkCreateGraphicsPipelines(GetImpl().device->logicalDevice, GetImpl().pipelineCache, 1, &pipelineCI, nullptr, &pipeline));



}

void Vulgine::GeneralPipeline::destroyImpl() {
    vkDestroyPipelineLayout(GetImpl().device->logicalDevice, pipelineLayout, nullptr);
    vkDestroyPipeline(GetImpl().device->logicalDevice, pipeline, nullptr);

    pipeline = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;

}

Vulgine::GeneralPipeline::~GeneralPipeline() {
    if(isCreated()) {
        vkDestroyPipelineLayout(GetImpl().device->logicalDevice, pipelineLayout, nullptr);
        vkDestroyPipeline(GetImpl().device->logicalDevice, pipeline, nullptr);

        pipeline = VK_NULL_HANDLE;
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void Vulgine::GeneralPipeline::bind(VkCommandBuffer cmdBuffer) const {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

bool Vulgine::PipelineKey::operator<(PipelineKey const& another) const {
    if(geometry < another.geometry)
        return true;
    if(geometry == another.geometry) {
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

void Vulgine::CompositionPipeline::createImpl() {
    std::vector<VkDescriptorSetLayout> layouts;

    layouts.push_back(renderPass->deferredLightingSubpass.compositionSet.layout());

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
            initializers::pipelineLayoutCreateInfo(
                    layouts.empty() ? nullptr : layouts.data(),
                    layouts.size());

    pPipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;

    // setting up arbitrary push constant range containing view Matrix of a camera

    VkPushConstantRange push_constant;
    //this push constant range starts at the beginning
    push_constant.offset = 0;
    //this push constant range takes up the size of a 4x4 matrix
    push_constant.size = sizeof(glm::mat4) + sizeof(glm::vec3);
    //this push constant range is accessible only in the fragment shader
    push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    pPipelineLayoutCreateInfo.pPushConstantRanges = &push_constant;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;

    VK_CHECK_RESULT(vkCreatePipelineLayout(GetImpl().device->logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));


    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
    VkPipelineRasterizationStateCreateInfo rasterizationState = initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE,0);
    VkPipelineColorBlendAttachmentState blendAttachmentState = initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
    VkPipelineColorBlendStateCreateInfo colorBlendState = initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
    VkPipelineDepthStencilStateCreateInfo depthStencilState = initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
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
    pipelineCI.pVertexInputState = emptyVertexState();
    pipelineCI.subpass = 1;


    // binding vertex shader

    auto const& vertexShaderName = "vert_background";

    if(GetImpl().vertexShaders.count(vertexShaderName) == 0){
        Utilities::ExitFatal(-1,"Error loading standard 'background.vert' shader");
    }
    auto& vertexShader = GetImpl().vertexShaders[vertexShaderName];


    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertexShader.module;
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].pName = "main";

    // binding fragment shader

    const char* defaultFragmentShader = "frag_composite";


    if(GetImpl().fragmentShaders.count(defaultFragmentShader) == 0){
        Utilities::ExitFatal(-1,"Error loading standard 'frag_composite' fragment shader");
    }
    auto& fragmentShader = GetImpl().fragmentShaders[defaultFragmentShader];


    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragmentShader.module;
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].pName = "main";


    VK_CHECK_RESULT(
            vkCreateGraphicsPipelines(GetImpl().device->logicalDevice, GetImpl().pipelineCache, 1, &pipelineCI, nullptr, &pipeline));

}

void Vulgine::CompositionPipeline::destroyImpl() {
    vkDestroyPipelineLayout(GetImpl().device->logicalDevice, pipelineLayout, nullptr);
    vkDestroyPipeline(GetImpl().device->logicalDevice, pipeline, nullptr);

    pipeline = VK_NULL_HANDLE;
    pipelineLayout = VK_NULL_HANDLE;

}

Vulgine::CompositionPipeline::~CompositionPipeline() {
    if(isCreated()) {
        vkDestroyPipelineLayout(GetImpl().device->logicalDevice, pipelineLayout, nullptr);
        vkDestroyPipeline(GetImpl().device->logicalDevice, pipeline, nullptr);

        pipeline = VK_NULL_HANDLE;
        pipelineLayout = VK_NULL_HANDLE;
    }
}

void Vulgine::CompositionPipeline::bind(VkCommandBuffer cmdBuffer) const {
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}
