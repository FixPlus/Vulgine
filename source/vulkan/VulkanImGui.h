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
#include <unordered_map>

namespace Vulgine{

    constexpr const int max_imgui_texture_count = 1024;

    class SamplerImpl;

    class GUI{

        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;


        std::vector<Memory::DynamicVertexBuffer> vertexBuffers;
        std::vector<Memory::DynamicIndexBuffer> indexBuffers;

        std::vector<int32_t> vertexCounts;
        std::vector<int32_t> indexCounts;


        VkDescriptorPool descriptorPool;

        /** all descriptors used by imgui have same layout*/

        VkDescriptorSetLayout descriptorSetLayout;

        /** one descriptor per image */

        std::unordered_map<Memory::Image*, std::pair<VkImageView, VkDescriptorSet>> descriptorSets;


        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline pipeline = VK_NULL_HANDLE;


        Memory::Image fontImage;
        VkDescriptorSet fontDescriptorSet;
        VkImageView fontView = VK_NULL_HANDLE;

        SamplerImpl* sampler;

        struct PushConstBlock {
            glm::vec2 scale;
            glm::vec2 translate;
        } pushConstBlock;

    public:

        uint32_t subpass = 0;

        bool visible = true;
        bool updated = false;
        float scale = 1.0f;


        void init(int numberOfFrames);

        void preparePipeline(VkRenderPass renderPass);

        bool update(int currentFrame);

        void draw(VkCommandBuffer commandBuffer, int currentFrame);

        void windowResized(uint32_t width, uint32_t height);

        void addTexturedImage(Memory::Image* image);

        void deleteTexturedImage(Memory::Image* image);

        void destroy();
    };
}
#endif //TEST_EXE_VULKANIMGUI_H
