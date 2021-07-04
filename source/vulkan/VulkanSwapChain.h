//
// Created by Бушев Дмитрий on 03.07.2021.
//

#ifndef TEST_EXE_VULKANSWAPCHAIN_H
#define TEST_EXE_VULKANSWAPCHAIN_H

#pragma once

#include <stdlib.h>
#include <string>
#include <assert.h>
#include <stdio.h>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
namespace Vulgine {
    typedef struct _SwapChainBuffers {
        VkImage image;
        VkImageView view;
    } SwapChainBuffer;

    class VulkanSwapChain {
    private:
        VkInstance instance;
        VkDevice device;
        VkPhysicalDevice physicalDevice;
        VkSurfaceKHR surface;
        // Function pointers
        PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
        PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
        PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
        PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
        PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
        PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
        PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
        PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
        PFN_vkQueuePresentKHR fpQueuePresentKHR;

        // Synchronization semaphores
        struct {
            // Swap chain image presentation
            VkSemaphore presentComplete;
            // Command buffer submission and execution
            VkSemaphore renderComplete;
        } semaphores;



        /** @brief Pipeline stages used to wait at for graphics queue submissions */
        const VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    public:
        VkSubmitInfo submitInfo;
        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        uint32_t imageCount;
        std::vector<VkImage> images;
        std::vector<SwapChainBuffer> buffers;
        uint32_t queueNodeIndex = UINT32_MAX;

        void initSurface(GLFWwindow *window);

        void connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

        void create(uint32_t *width, uint32_t *height, bool vsync = false);

        VkResult acquireNextImage(uint32_t *imageIndex);

        VkResult queuePresent(VkQueue queue, uint32_t imageIndex);

        void cleanup();
    };
}
#endif //TEST_EXE_VULKANSWAPCHAIN_H
