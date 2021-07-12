//
// Created by Бушев Дмитрий on 12.07.2021.
//

#ifndef TEST_EXE_VULKANIMGUI_H
#define TEST_EXE_VULKANIMGUI_H

#include "imgui/imgui.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <vulkan/VulkanAllocatable.h>
#include <glm/vec2.hpp>
#include <vulkan/VulkanSampler.h>

namespace Vulgine{


    class GUI{

        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        uint32_t subpass = 0;

        std::vector<Memory::DynamicVertexBuffer> vertexBuffers;
        std::vector<Memory::DynamicIndexBuffer> indexBuffers;

        std::vector<int32_t> vertexCounts;
        std::vector<int32_t> indexCounts;


        VkDescriptorPool descriptorPool;
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSet descriptorSet;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;


        Memory::Image fontImage;
        VkImageView fontView = VK_NULL_HANDLE;
        Sampler sampler;

        struct PushConstBlock {
            glm::vec2 scale;
            glm::vec2 translate;
        } pushConstBlock;

    public:

        bool visible = true;
        bool updated = false;
        float scale = 1.0f;


        void init(int numberOfFrames);

        void preparePipeline(VkRenderPass renderPass);

        bool update(int currentFrame);

        void draw(VkCommandBuffer commandBuffer, int currentFrame);

        void windowResized(uint32_t width, uint32_t height);

        void destroy();
    };
}
#endif //TEST_EXE_VULKANIMGUI_H