#include "Vulgine.h"
#include "vulkan/VulkanDebug.h"
#include "VulgineScene.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <string>
#include <sstream>
#include <Utilities.h>
#include <vector>

namespace Vulgine{

    VulgineImpl* vlg_instance = nullptr;
    std::string err_message = "";

    Initializers initializeInfo;

    VulgineImpl::VulgineImpl(){

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::cout << extensionCount << " extensions supported\n";

    }

    bool VulgineImpl::cycle() {
        if(glfwWindowShouldClose(window.instance))
            return false;
        glfwPollEvents();

        if(prepared)
            renderFrame();

        return true;
    }

    VulgineImpl::~VulgineImpl() {

        debug::freeDebugCallback(instance);
        swapChain.cleanup();

        offScreenFramebuffers.clear();
        onScreenFramebuffers.clear();

        vkDestroyImageView(device->logicalDevice, depthStencil.view, nullptr);
        vkDestroyImage(device->logicalDevice, depthStencil.image, nullptr);
        vkFreeMemory(device->logicalDevice, depthStencil.mem, nullptr);

        destroyCommandBuffers();

        destroyRenderPasses();

        vkDestroyCommandPool(device->logicalDevice, cmdPool, nullptr);

        delete device;
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window.instance);
        glfwTerminate();
    }

    void VulgineImpl::destroyCommandBuffers()
    {
        vkFreeCommandBuffers(device->logicalDevice, cmdPool, static_cast<uint32_t>(drawCmdBuffers.size()), drawCmdBuffers.data());
        drawCmdBuffers.clear();
    }


    std::string getLastErrorLog(){
        return err_message;
    }

    void error_callback(int code, const char* description){
        errs("GLFW error: " + std::string(description) + "(Error code: " + std::to_string(code) + ")");
    }

    Vulgine* Vulgine::createInstance(){

        // if no specific log file is set, proceed using standard output file

        if(logger.hasNullLogFile())
            logger.changeLogFile(&std::cout);

        if(vlg_instance == nullptr){

            // check version compatibility

            int major, minor, revision;

            int h_major, h_minor, h_revision;

            getVersion(&major, &minor, &revision);
#if 0
            getHeaderVersion(&h_major, &h_minor, &h_revision);

            if(major != h_major){
                errs("Loaded VulGine library version is incompatible: expected " + std::to_string(h_major) + ", loaded " + std::to_string(major));
                return nullptr;
            }

            if(minor < h_minor){
                errs("Loaded VulGine library version is incompatible: expected not less " +
                        std::to_string(major) + "." + std::to_string(h_minor) + ", loaded " + std::to_string(major) + "." + std::to_string(minor));
                return nullptr;
            }
#endif
            logger("Loaded compatible VulGine version: " + getStringVersion());

            // checking if loaded GLFW library has compatible version


            glfwGetVersion(&major, &minor, &revision);
            if(major != GLFW_VERSION_MAJOR) {
                errs("Unsupported GLFW version: " + std::to_string(major) + "(expected " +
                        std::to_string(GLFW_VERSION_MAJOR) + ")");
                return nullptr;
            }
            if(minor < GLFW_VERSION_MINOR){
                errs("Incompatible GLFW version: " + std::to_string(major) + "." + std::to_string(minor)
                        + "(minimum required " + std::to_string(GLFW_VERSION_MAJOR) + "." +std::to_string(GLFW_VERSION_MINOR) +")");
                return nullptr;
            }
            logger("Proper GLFW version loaded: " + std::string(glfwGetVersionString()));

            VulgineImpl* vlg_instance_impl;
            try{
                vlg_instance_impl = new VulgineImpl();
                vlg_instance = vlg_instance_impl;
            }
            catch (std::bad_alloc const& e){
                vlg_instance = nullptr;
                errs("Can't create VulGine instance: out of RAM");
                return nullptr;
            }
            logger("VulGine Instance allocated");

            bool initComplete = vlg_instance_impl->initialize();

            if(!initComplete){
                delete vlg_instance;
                return nullptr;
            }
            return vlg_instance;

        }else{
            errs("Vulgine Instance has been already created");
            return nullptr;
        }


    }

    void Vulgine::freeInstance(Vulgine* instance){
        if(instance == nullptr || instance != vlg_instance)
            errs("Invalid pointer");
        else {
            delete vlg_instance;
            vlg_instance = nullptr;
            logger("VulGine instance freed");
        }
    }

    bool VulgineImpl::initialize() {

        initFields();

        // initializing GLFW

        glfwInit();

        int code = glfwGetError(nullptr);

        if (code != GLFW_NO_ERROR){
            errs("GLFW Initialization failed with following error code: " + std::to_string(code));
            return false;
        }
        logger("GLFW Initialized");

        glfwSetErrorCallback(error_callback);


        // creating window

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        window.instance = glfwCreateWindow(window.width, window.height,
                                       window.name.c_str(), nullptr, nullptr);


        logger("GLFW: created window");


        //TODO: vulkan instance, physical and virtual devices initialization processes

        createVkInstance();

        logger("Vulkan Instance created");

        if(initializeInfo.enableVulkanValidationLayers){
            // The report flags determine what type of messages for the layers will be displayed
            // For validating (debugging) an application the error and warning bits should suffice
            VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
            // Additional flags include performance info, loader and layer debug messages, etc.
            debug::setupDebugging(instance, debugReportFlags, VK_NULL_HANDLE);

            logger("Vulkan Validation layers enabled");
        }

        createVulkanDevice();

        logger("Vulkan Device Created");

        createCommandPool();

        swapChain.create(&window.width,&window.height, true);

        logger("Swap chain created");

        createCommandBuffers();

        setupDepthStencil();

        return true;
    }

    void VulgineImpl::createCommandPool()
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VK_CHECK_RESULT(vkCreateCommandPool(device->logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
    }

    void VulgineImpl::createCommandBuffers()
    {
        // Create one command buffer for each swap chain image and reuse for rendering
        drawCmdBuffers.resize(swapChain.imageCount);

        VkCommandBufferAllocateInfo cmdBufAllocateInfo =
                initializers::commandBufferAllocateInfo(
                        cmdPool,
                        VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                        static_cast<uint32_t>(drawCmdBuffers.size()));

        VK_CHECK_RESULT(vkAllocateCommandBuffers(device->logicalDevice, &cmdBufAllocateInfo, drawCmdBuffers.data()));
    }

    void VulgineImpl::createVkInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = initializeInfo.applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VulGine";
        appInfo.engineVersion = VK_MAKE_VERSION(VULGINE_VERSION_MAJOR, VULGINE_VERSION_MINOR, VULGINE_VERSION_REVISION);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto glfwExtensions = getRequiredExtensionsList();

        if(initializeInfo.enableVulkanValidationLayers){
            // The VK_LAYER_KHRONOS_validation contains all current validation functionality.
            // Note that on Android this layer requires at least NDK r20
            const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
            // Check if this layer is available at instance level
            uint32_t instanceLayerCount;
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
            std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
            vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());
            bool validationLayerPresent = false;
            for (VkLayerProperties layer : instanceLayerProperties) {
                if (strcmp(layer.layerName, validationLayerName) == 0) {
                    validationLayerPresent = true;
                    break;
                }
            }
            if (validationLayerPresent) {
                createInfo.ppEnabledLayerNames = &validationLayerName;
                createInfo.enabledLayerCount = 1;
            } else {
                createInfo.enabledLayerCount = 0;
                initializeInfo.enableVulkanValidationLayers = false;
                errs("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
            }
        }


        createInfo.enabledExtensionCount = glfwExtensions.size();
        createInfo.ppEnabledExtensionNames = glfwExtensions.data();

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance))
    }

    void VulgineImpl::initFields() {

        if(initializeInfo.windowSize) {
            auto size = initializeInfo.windowSize.value();
            window.height = size.second;
            window.width = size.first;
        }

        if(initializeInfo.windowName){
            window.name = initializeInfo.windowName.value();
        }

    }

    std::vector<const char*> VulgineImpl::getRequiredExtensionsList() {

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);


        if(initializeInfo.enableVulkanValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    void VulgineImpl::createVulkanDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if(deviceCount == 0){
            Utilities::ExitFatal(-1, "There are no GPUs supporting Vulkan");
        }

        std::vector<VkPhysicalDevice> availableDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, availableDevices.data());

        //TODO: make it available to choose the device



        device = new VulkanDevice(availableDevices[0]);
        VkResult res = device->createLogicalDevice(enabledFeatures, enabledDeviceExtensions, deviceCreatepNextChain);
        if (res != VK_SUCCESS) {
            Utilities::ExitFatal(res, "Could not create Vulkan device: \n" + Utilities::errorString(res));
        }

        if(!getSupportedDepthFormat()){
            Utilities::ExitFatal(-1, "Selected GPU doesn't support any depth format");
        }

        // Get a graphics queue from the device
        vkGetDeviceQueue(device->logicalDevice, device->queueFamilyIndices.graphics, 0, &queue);

        swapChain.connect(instance, device->physicalDevice, device->logicalDevice);

        swapChain.initSurface(window.instance);

    }

    void VulgineImpl::renderFrame() {

        // Acquire the next image from the swap chain
        VkResult result = swapChain.acquireNextImage(&currentBuffer);
        // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
            // TODO: recreate swap chain here as window size changed
        }
        else {
            VK_CHECK_RESULT(result);
        }

        swapChain.submitInfo.commandBufferCount = 1;
        swapChain.submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &swapChain.submitInfo, VK_NULL_HANDLE));


        result = swapChain.queuePresent(queue, currentBuffer);
        if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
            if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                // TODO: recreate swap chain here as window size changed
                return;
            } else {
                VK_CHECK_RESULT(result);
            }
        }

        // TODO: add support for rendering frames 'in flight' to improve graphics card resource usage

        VK_CHECK_RESULT(vkQueueWaitIdle(queue));

    }

    Scene *VulgineImpl::initNewScene() {
        Scene* ret;
        try{
            ret = new SceneImpl{};
        }
        catch(std::bad_alloc const& e) {
            Utilities::ExitFatal(-1, e.what());
        }

        return ret;
    }

    RenderTarget *VulgineImpl::initNewRenderTarget() {
        return nullptr;
    }

    void VulgineImpl::buildRenderPass(const std::vector<RenderTask> &renderTaskQueue) {

    }

    VkBool32 VulgineImpl::getSupportedDepthFormat()
    {
        // Since all depth formats may be optional, we need to find a suitable depth format to use
        // Start with the highest precision packed format
        std::vector<VkFormat> depthFormats = {
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_FORMAT_D32_SFLOAT,
                VK_FORMAT_D24_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM_S8_UINT,
                VK_FORMAT_D16_UNORM
        };

        for (auto& format : depthFormats)
        {
            VkFormatProperties formatProps;
            vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProps);
            // Format must support depth stencil attachment for optimal tiling
            if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            {
                depthFormat = format;
                return true;
            }
        }

        return false;
    }

    void VulgineImpl::setupDepthStencil()
    {
        VkImageCreateInfo imageCI{};
        imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCI.imageType = VK_IMAGE_TYPE_2D;
        imageCI.format = depthFormat;
        imageCI.extent = { window.width, window.height, 1 };
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
        imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

        VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCI, nullptr, &depthStencil.image));
        VkMemoryRequirements memReqs{};
        vkGetImageMemoryRequirements(device->logicalDevice, depthStencil.image, &memReqs);

        VkMemoryAllocateInfo memAllloc{};
        memAllloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAllloc.allocationSize = memReqs.size;
        memAllloc.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllloc, nullptr, &depthStencil.mem));
        VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, depthStencil.image, depthStencil.mem, 0));

        VkImageViewCreateInfo imageViewCI{};
        imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCI.image = depthStencil.image;
        imageViewCI.format = depthFormat;
        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;
        imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT
        if (depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT) {
            imageViewCI.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &imageViewCI, nullptr, &depthStencil.view));
    }

    void VulgineImpl::destroyRenderPasses() {
        for(auto* pass: renderPasses)
            delete pass;

        renderPasses.clear();
    }

    void disableLog(){
        logger.disable();
    }
    void enableLog(){
        logger.enable();
    }
    void redirectLogTo(std::ostream& output){
        logger.changeLogFile(&output);
    }

    void getVersion(int* v_major, int* v_minor, int* v_revision){
        *v_major = VULGINE_VERSION_MAJOR;
        *v_minor = VULGINE_VERSION_MINOR;
        *v_revision = VULGINE_VERSION_REVISION;
    }

    void Initializers::reset() {
        *this = Initializers();
    }

    std::string getStringVersion() {
        std::string ret;
        ret = "VulGine " + std::to_string(VULGINE_VERSION_MAJOR) + "." + std::to_string(VULGINE_VERSION_MINOR) + "." + std::to_string(VULGINE_VERSION_REVISION);
        return ret;
    }


}
