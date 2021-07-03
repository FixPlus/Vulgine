#ifndef VULGINE_VULGINE_H
#define VULGINE_VULGINE_H
#include "../include/IVulgine.h"
#include "vulkan/VulkanDevice.h"
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulgine {


    class VulgineImpl: public Vulgine{
        static std::vector<const char*> getRequiredExtensionsList();
    protected:
        struct Window {
            GLFWwindow *instance = nullptr;
            std::string name = "VulGine App";
            uint32_t width = 800;
            uint32_t height = 600;
        } window;

        VkInstance instance = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkPhysicalDeviceFeatures deviceFeatures;
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

        /** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
        VkPhysicalDeviceFeatures enabledFeatures{};
        /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
        std::vector<const char*> enabledDeviceExtensions;

        /** @brief Optional pNext structure for passing extension structures to device creation */
        void* deviceCreatepNextChain = nullptr;

        VulkanDevice* device;

        void createVkInstance();
        void createVulkanDevice();

        void initFields();
    public:
        bool initialize();
        explicit VulgineImpl();
        ~VulgineImpl() override;
        bool cycle() override;

    };


}

#endif //VULGINE_VULGINE_H
