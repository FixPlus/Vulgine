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
#include "vulkan/VulkanInitializers.hpp"
#include "imgui/imgui.h"
#include <thread>

namespace {
    Vulgine::VulgineImpl impl;
    bool impl_created = false;
}

namespace Vulgine{

    bool Init(){
        // if no specific log file is set, proceed using standard output file

        if(logger.hasNullLogFile())
            logger.changeLogFile(&std::cout);

        if(!impl_created){
            impl_created = true;

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
                return false;
            }
            if(minor < GLFW_VERSION_MINOR){
                errs("Incompatible GLFW version: " + std::to_string(major) + "." + std::to_string(minor)
                     + "(minimum required " + std::to_string(GLFW_VERSION_MAJOR) + "." +std::to_string(GLFW_VERSION_MINOR) +")");
                return false;
            }
            logger("Proper GLFW version loaded: " + std::string(glfwGetVersionString()));

            ObjectImpl::fillTypeNameTable();


            bool initComplete = impl.initialize();

            return initComplete;

        }else{
            errs("Vulgine Instance has been already created");
            return false;
        }
    }

    Vulgine* Get(){
        if(impl_created)
            return &impl;
        else
            return nullptr;
    }

    VulgineImpl& GetImpl(){
        return impl;
    }

    void Terminate(){
        if(impl_created) {
            impl.terminate();
            impl_created = false;
        }
    }

    std::map<GLFWwindow *, VulgineImpl::Window*> VulgineImpl::Window::windowMap;

    std::string err_message = "";

    Initializers initializeInfo;



    bool VulgineImpl::cycle() {

        if (glfwWindowShouldClose(window.instance())) {
            VK_CHECK_RESULT(vkQueueWaitIdle(queue));
            return false;
        }

        timeMarkers.tEnd = std::chrono::high_resolution_clock::now();
        auto frameTime = std::chrono::duration<double, std::milli>(timeMarkers.tEnd - timeMarkers.tStart).count();
        double deltaT = frameTime / 1000.0f;

        timeMarkers.tStart = std::chrono::high_resolution_clock::now();

        fpsCounter.update(deltaT);

        keyboardState.keysPressed.clear();

        glfwPollEvents();

        updateGUI();

        if (prepared)
            renderFrame();

        return true;
    }

    void VulgineImpl::terminate() {

        scenes.clear();
        materials.clear();
        images.clear();
        uniformBuffers.clear();
        samplers.clear();



        gui.destroy();

        destroyShaders();


        pipelineMap.clear();

        debug::freeDebugCallback(instance);

        destroySyncPrimitives();

        swapChain.cleanup();


        vkDestroyImageView(device->logicalDevice, depthStencil.view, nullptr);
        vkDestroyImage(device->logicalDevice, depthStencil.image, nullptr);
        vkFreeMemory(device->logicalDevice, depthStencil.mem, nullptr);

        if(msaaImage.image.allocated){
            vkDestroyImageView(device->logicalDevice, msaaImage.view, nullptr);
            msaaImage.image.free();
        }

        destroyDescriptorPools();

        destroyCommandBuffers();

        destroyRenderPasses();

        vkDestroyPipelineCache(device->logicalDevice, pipelineCache, nullptr);

        vkDestroyCommandPool(device->logicalDevice, cmdPool, nullptr);

        vmaDestroyAllocator(allocator);

        delete device;
        vkDestroyInstance(instance, nullptr);

        window.terminate();

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


        window.init();

        glfwGetCursorPos(window.instance(), &mouseState.cursor.posX, &mouseState.cursor.posY);

        logger("GLFW: created window");


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

        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2;
        allocatorInfo.physicalDevice = device->physicalDevice;
        allocatorInfo.device = device->logicalDevice;
        allocatorInfo.instance = instance;

        VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &allocator))

        logger("Vulkan Device Created");

        createCommandPool();

        setupSwapChain();

        createSyncPrimitives();

        logger("Swap chain created");

        createCommandBuffers();

        setupDepthStencil();

        logger("Command buffer and depth stencil image allocated");

        createPipelineCache();

        logger("Created pipeline cache");

        loadShaders();

        logger("default shader pack loaded");

        setupDescriptorPools();

        gui.init(swapChain.imageCount);

        gui.windowResized(window.width, window.height);



        logger("Initialization of VulGine completed successfully");


        return true;
    }

    void VulgineImpl::createCommandPool()
    {
        VkCommandPoolCreateInfo cmdPoolInfo = {};
        cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmdPoolInfo.queueFamilyIndex = swapChain.queueNodeIndex;
        cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
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
        appInfo.apiVersion = VK_API_VERSION_1_2;

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


        auto size = initializeInfo.windowSize;
        window.height = size.second;
        window.width = size.first;
        window.name = initializeInfo.windowName;
        window.title = initializeInfo.windowName;

        settings.vsync = initializeInfo.vsync;
        window.fullscreen = initializeInfo.fullscreen;


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

        // Get a transfer queue from the device

        vkGetDeviceQueue(device->logicalDevice, device->queueFamilyIndices.transfer, 0, &transferQueue);

        swapChain.connect(instance, device->physicalDevice, device->logicalDevice);

        swapChain.initSurface(window.instance());

    }

    void VulgineImpl::renderFrame() {
        if(window.resized)
            windowResize();


        vkWaitForFences(device->logicalDevice, 1, &framesSync[currentFrame].inFlightSync, VK_TRUE, UINT64_MAX);


        // Acquire the next image from the swap chain
        VkResult result = swapChain.acquireNextImage(&currentBuffer, framesSync[currentFrame].presentComplete);
        // Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
        if ((result == VK_ERROR_OUT_OF_DATE_KHR) || (result == VK_SUBOPTIMAL_KHR)) {
            windowResize();
        }
        else {
            VK_CHECK_RESULT(result);
        }

        onCycle();

        if(gui.update(currentBuffer))
            cmdBuffersOutdated = true;

        // synchronize dynamic buffers data

        uniformBuffers.iterate([](UniformBufferImpl& buffer){ buffer.sync();});
        scenes.iterate([](SceneImpl& scene){ scene.lightUBO.sync();});

        buildCommandBuffers(currentBuffer);

        if (swapChainFences[currentBuffer] != VK_NULL_HANDLE) {
            vkWaitForFences(device->logicalDevice, 1, &swapChainFences[currentBuffer], VK_TRUE, UINT64_MAX);
        }
        // Mark the image as now being in use by this frame
        swapChainFences[currentBuffer] = framesSync[currentFrame].inFlightSync;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];

        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &framesSync[currentFrame].presentComplete;
        submitInfo.pSignalSemaphores = &framesSync[currentFrame].renderComplete;

        vkResetFences(device->logicalDevice, 1, &framesSync[currentFrame].inFlightSync);


        VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, framesSync[currentFrame].inFlightSync));


        //if(lastBuffer != -1) {
            result = swapChain.queuePresent(queue, currentBuffer, framesSync[currentFrame].renderComplete);
            if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
                if (result == VK_ERROR_OUT_OF_DATE_KHR) {
                    windowResize();
                    return;
                } else {
                    VK_CHECK_RESULT(result);
                }
            }
        //}



        currentFrame = (currentFrame + 1) % settings.framesInFlight;



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
        imageCI.extent = { vieportInfo.width, vieportInfo.height, 1 };
        imageCI.mipLevels = 1;
        imageCI.arrayLayers = 1;
        imageCI.samples = settings.msaa;
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
        renderPasses.clear();
    }

    Scene *VulgineImpl::initNewScene() {
        return scenes.emplace();
    }

    void VulgineImpl::deleteScene(Scene *scene) {
        scenes.free(dynamic_cast<SceneImpl*>(scene));
    }

    Material *VulgineImpl::initNewMaterial() {
        return materials.emplace();
    }

    void VulgineImpl::deleteMaterial(Material *material) {
       materials.free(dynamic_cast<MaterialImpl*>(material));
    }


    void VulgineImpl::buildRenderPass(RenderPassImpl* renderPass){

        renderPass->destroy();

        CHECK_DEVICE_LIMITS(renderPass)

        if(renderPass->onscreen) { // TODO: generalize onscreen render pass creation

            std::array<VkAttachmentDescription, 3> attachments = {};

            // Color attachment
            attachments[0].format = swapChain.colorFormat;
            attachments[0].samples = settings.msaa;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = settings.msaa > 1 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
                                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            // Depth attachment
            attachments[1].format = depthFormat;
            attachments[1].samples = settings.msaa;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            // Reslove color attachment for multisampling image

            if (settings.msaa > 1) {
                attachments[2].format = swapChain.colorFormat;
                attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }

            VkAttachmentReference colorReference = {};
            colorReference.attachment = 0;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthReference = {};
            depthReference.attachment = 1;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentResolveRef{};
            colorAttachmentResolveRef.attachment = 2;
            colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments = &colorReference;
            subpassDescription.pDepthStencilAttachment = &depthReference;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = nullptr;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = nullptr;
            subpassDescription.pResolveAttachments = settings.msaa > 1 ? &colorAttachmentResolveRef : nullptr;

            // Subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies{};

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(settings.msaa > 1 ? attachments.size() :
                                                                   attachments.size() - 1);
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VK_CHECK_RESULT(
                    vkCreateRenderPass(device->logicalDevice, &renderPassInfo, nullptr, &renderPass->renderPass));
        } else{


            if(renderPass->frameBuffer.attachmentsImages.empty()){
                Utilities::ExitFatal(-1, "Cannot create render pass with empty attachments");
            }

            std::vector<VkAttachmentDescription> attachments = {};
            std::optional<VkAttachmentReference> depthAttachemntRef;
            std::vector<VkAttachmentReference> colorAttachmentRefs{};
            attachments.resize(renderPass->frameBuffer.attachmentCount());

            for(auto& it: renderPass->frameBuffer.attachmentsImages) {
                auto binding = it.first;
                auto& image = it.second;


                attachments.at(binding).format = image.createInfo.format;
                attachments.at(binding).samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling for offscreen rendering yet
                attachments.at(binding).loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachments.at(binding).storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments.at(binding).stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments.at(binding).stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                // for now we consider it to be shader read-only optimal

                attachments.at(binding).initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                // for now we consider it to be used as sampled image

                attachments.at(binding).finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                if(image.createInfo.format == depthFormat){
                    depthAttachemntRef.emplace();
                    depthAttachemntRef->attachment = binding;
                    depthAttachemntRef->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                } else{
                    auto& colRef = colorAttachmentRefs.emplace_back();
                    colRef.attachment = binding;
                    colRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
            }


            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = colorAttachmentRefs.size();
            subpassDescription.pColorAttachments = colorAttachmentRefs.data();
            subpassDescription.pDepthStencilAttachment = depthAttachemntRef.has_value() ? &depthAttachemntRef.value() : nullptr;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = nullptr;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = nullptr;
            subpassDescription.pResolveAttachments = nullptr;

            // Subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies{};

            // TODO: deduct external dependencies by framebuffer images use info

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = attachments.size();

            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VK_CHECK_RESULT(
                    vkCreateRenderPass(device->logicalDevice, &renderPassInfo, nullptr, &renderPass->renderPass));

            renderPass->frameBuffer.renderPass = renderPass;
            renderPass->frameBuffer.create();
        }


        renderPassLine.push_front(renderPass);

        renderPass->create();

        for(auto* pass: renderPass->dependencies)
            buildRenderPass(dynamic_cast<RenderPassImpl*>(pass));

    }

    void VulgineImpl::buildRenderPasses() {

        // Step 1: find onscreen render pass

        int onscreenRenderPassCount = 0;
        RenderPassImpl* found = nullptr;
        renderPassLine.clear();

        renderPasses.iterate([&onscreenRenderPassCount, &found](RenderPassImpl& renderPass)
        {
            if(renderPass.onscreen) {
                found = &renderPass;
                onscreenRenderPassCount++;
            }
        });

        if(onscreenRenderPassCount != 1){
            Utilities::ExitFatal(-1, "VulGine expects exactly one onscreen render pass");
        }

        onscreenRenderPass = found;

        buildRenderPass(onscreenRenderPass); // TODO: parse dependency graph to detect loops

        //recreate on screen framebuffers

        destroyOnscreenFrameBuffers();
        createOnscreenFrameBuffers();

        // every pipeline should be recreated here because it's state depends on render pass state

        pipelineMap.clear();

        gui.preparePipeline(onscreenRenderPass->renderPass);

        // lastly, we build draw command buffers for each swap chain image

        for(int i = 0; i < swapChain.imageCount; i++)
            buildCommandBuffers(i);

    }

    void VulgineImpl::createPipelineCache()
    {
        VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
        pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        VK_CHECK_RESULT(vkCreatePipelineCache(device->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache));
    }

    void VulgineImpl::createOnscreenFrameBuffers() {

        if(onscreenRenderPass == nullptr)
            return;

        auto& frameBuffer = onscreenRenderPass->frameBuffer;

        frameBuffer.destroy();


        if(settings.msaa > 1){

            // TODO: move this somewhere else

            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = static_cast<uint32_t>(vieportInfo.width);
            imageInfo.extent.height = static_cast<uint32_t>(vieportInfo.height);
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;

            imageInfo.format = swapChain.colorFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.samples = settings.msaa;
            imageInfo.flags = 0; // Optional


            msaaImage.image.allocate(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, 0, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VkImageViewCreateInfo viewCreateInfo = {};
            viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;


            viewCreateInfo.format = imageInfo.format;
            viewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
            viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
            // Linear tiling usually won't support mip maps
            // Only set mip map count if optimal tiling is used
            viewCreateInfo.subresourceRange.levelCount = 1;
            viewCreateInfo.image = msaaImage.image.image;
            VK_CHECK_RESULT(vkCreateImageView(GetImpl().device->logicalDevice, &viewCreateInfo, nullptr, &msaaImage.view));

        }

        frameBuffer.width = vieportInfo.width;
        frameBuffer.height = vieportInfo.height;



        std::vector<VkImageView> views{};
        if(settings.msaa > 1) {

            // color attachment (binding 0)

            for (int i = 0; i < swapChain.imageCount; i++) {
                views.push_back(msaaImage.view);
            }

            frameBuffer.addOnsrceenAttachment(views);

            views.clear();

            // depth attachment (binding 1)

            for (int i = 0; i < swapChain.imageCount; i++) {
                views.push_back(depthStencil.view);
            }

            frameBuffer.addOnsrceenAttachment(views);

            views.clear();

            // resolved color attachment (binding 2)

            for (int i = 0; i < swapChain.imageCount; i++) {
                views.push_back(swapChain.buffers[i].view);
            }

            frameBuffer.addOnsrceenAttachment(views);

        } else {
            // color attachment (binding 0)

            for (int i = 0; i < swapChain.imageCount; i++) {
                views.push_back(swapChain.buffers[i].view);
            }

            frameBuffer.addOnsrceenAttachment(views);

            views.clear();

            // depth attachment (binding 1)

            for (int i = 0; i < swapChain.imageCount; i++) {
                views.push_back(depthStencil.view);
            }

            frameBuffer.addOnsrceenAttachment(views);

        }

        frameBuffer.renderPass = onscreenRenderPass;

        frameBuffer.create();

    }

    void VulgineImpl::destroyOnscreenFrameBuffers() {
        if(onscreenRenderPass == nullptr)
            return;

        if(msaaImage.image.allocated){
            vkDestroyImageView(device->logicalDevice, msaaImage.view, nullptr);
            msaaImage.image.free();
        }

        onscreenRenderPass->frameBuffer.destroy();
    }

    VkShaderModule loadShader(const char *fileName, VkDevice device)
    {
        std::ifstream is(fileName, std::ios::binary | std::ios::in | std::ios::ate);

        if (is.is_open())
        {
            size_t size = is.tellg();
            is.seekg(0, std::ios::beg);
            char* shaderCode = new char[size];
            is.read(shaderCode, size);
            is.close();

            assert(size > 0);

            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo moduleCreateInfo{};
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.codeSize = size;
            moduleCreateInfo.pCode = (uint32_t*)shaderCode;

            VK_CHECK_RESULT(vkCreateShaderModule(device, &moduleCreateInfo, NULL, &shaderModule));

            delete[] shaderCode;

            return shaderModule;
        }
        else
        {
            Utilities::ExitFatal( -1, std::string("Error: Could not open shader file \"") + fileName + "\"" + "\n");
            return VK_NULL_HANDLE;
        }
    }

    void VulgineImpl::loadShaders() {
        auto shader = loadShader("default.vert.spv", device->logicalDevice);
        vertexShaders.emplace(std::piecewise_construct, std::forward_as_tuple("vert_default"), std::forward_as_tuple(shader, "vert_default"));
        shader = loadShader("default.frag.spv", device->logicalDevice);
        fragmentShaders.emplace(std::piecewise_construct, std::forward_as_tuple("frag_default"), std::forward_as_tuple(shader, "frag_default"));

        shader = loadShader("color.vert.spv", device->logicalDevice);
        vertexShaders.emplace(std::piecewise_construct, std::forward_as_tuple("vert_color"), std::forward_as_tuple(shader, "vert_color"));
        shader = loadShader("color.frag.spv", device->logicalDevice);
        fragmentShaders.emplace(std::piecewise_construct, std::forward_as_tuple("frag_color"), std::forward_as_tuple(shader, "frag_color"));

        shader = loadShader("uioverlay.vert.spv", device->logicalDevice);
        vertexShaders.emplace(std::piecewise_construct, std::forward_as_tuple("vert_imgui"), std::forward_as_tuple(shader, "vert_imgui"));
        shader = loadShader("uioverlay.frag.spv", device->logicalDevice);
        fragmentShaders.emplace(std::piecewise_construct, std::forward_as_tuple("frag_imgui"), std::forward_as_tuple(shader, "frag_imgui"));

        shader = loadShader("background.vert.spv", device->logicalDevice);
        vertexShaders.emplace(std::piecewise_construct, std::forward_as_tuple("vert_background"), std::forward_as_tuple(shader, "vert_background"));
        shader = loadShader("background.frag.spv", device->logicalDevice);
        fragmentShaders.emplace(std::piecewise_construct, std::forward_as_tuple("frag_background"), std::forward_as_tuple(shader, "frag_background"));

        shader = loadShader("backgroundTextured.frag.spv", device->logicalDevice);
        fragmentShaders.emplace(std::piecewise_construct, std::forward_as_tuple("frag_background_textured"), std::forward_as_tuple(shader, "frag_background_textured"));

    }

    void VulgineImpl::destroyShaders() {
        vertexShaders.clear();
        fragmentShaders.clear();
    }

    void VulgineImpl::buildCommandBuffers(int imageIndex) {


        VkCommandBufferBeginInfo cmdBufInfo = initializers::commandBufferBeginInfo();

        VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[imageIndex], &cmdBufInfo))

        for(auto* renderPass: renderPassLine) {
            assert(renderPass->camera && "RenderPass must have bounded camera");
            assert(renderPass->scene && "RenderPass must have bounded scene");

            renderPass->begin(drawCmdBuffers[imageIndex], imageIndex);

            renderPass->buildCmdBuffers(drawCmdBuffers[imageIndex], imageIndex);

            if(renderPass->onscreen)
                gui.draw(drawCmdBuffers[imageIndex], imageIndex);

            renderPass->end(drawCmdBuffers[imageIndex]);


        }

        VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[imageIndex]));

        cmdBuffersOutdated = false;

        prepared = true;
    }

    void VulgineImpl::setupSwapChain(){
        swapChain.create(&vieportInfo.width, &vieportInfo.height, settings.vsync);
    }

    void VulgineImpl::windowResize() {
        if(!prepared)
            return;

        prepared = false;

        // Ensure all operations on the device have been finished before destroying resources
        vkDeviceWaitIdle(device->logicalDevice);

        // Recreate swap chain
        vieportInfo.width = window.width;
        vieportInfo.height = window.height;
        setupSwapChain();

        // Recreate the frame buffers
        vkDestroyImageView(device->logicalDevice, depthStencil.view, nullptr);
        vkDestroyImage(device->logicalDevice, depthStencil.image, nullptr);
        vkFreeMemory(device->logicalDevice, depthStencil.mem, nullptr);
        setupDepthStencil();

        destroyOnscreenFrameBuffers();
        createOnscreenFrameBuffers();


        // Command buffers need to be recreated as they may store
        // references to the recreated frame buffer
        destroyCommandBuffers();
        createCommandBuffers();
        for(int i = 0; i < swapChain.imageCount; ++i)
            buildCommandBuffers(i);

        vkDeviceWaitIdle(device->logicalDevice);

        gui.windowResized(window.width, window.height);

        window.resized = false;

    }

    void VulgineImpl::keyDown(VulgineImpl::Window *window, int key) {
        if(!ImGui::GetIO().WantCaptureKeyboard) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    guiImpl.opened = false;
                    break;
                case GLFW_KEY_E:
                    if (window->fullscreen) window->goWindowed();
                    else window->goFullscreen();
                    break;
                case GLFW_KEY_GRAVE_ACCENT:
                    mouseState.enableCursor();
                    guiImpl.opened = !guiImpl.opened;
                    break;
                default:
                    break;
            }
        }

        keyboardState.keysPressed.emplace(key, 0);
        keyboardState.keysDown.emplace(key, 0);
        if(!ImGui::GetIO().WantCaptureKeyboard)
            keyboardState.onKeyDown(key);
    }

    void VulgineImpl::keyUp(VulgineImpl::Window *window, int key) {
        keyboardState.keysDown.erase(key);
        if(!ImGui::GetIO().WantCaptureKeyboard)
            keyboardState.onKeyUp(key);
    }

    double VulgineImpl::lastFrameTime() const {
        return fpsCounter.lastFrameTime;
    }

    void VulgineImpl::createSyncPrimitives() {

        assert(settings.framesInFlight <= swapChain.imageCount && "FIF count must be less or equal to number of swap chain image buffers");

        framesSync.resize(settings.framesInFlight);

        VkSemaphoreCreateInfo semaphoreCreateInfo = initializers::semaphoreCreateInfo();

        VkFenceCreateInfo fenceCreateInfo = initializers::fenceCreateInfo();
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for(auto& frameSync: framesSync) {
            // Create a semaphore used to synchronize image presentation
            // Ensures that the image is displayed before we start submitting new commands to the queue
            VK_CHECK_RESULT(vkCreateSemaphore(device->logicalDevice, &semaphoreCreateInfo, nullptr,
                                              &frameSync.presentComplete));
            // Create a semaphore used to synchronize command submission
            // Ensures that the image is not presented until all commands have been submitted and executed
            VK_CHECK_RESULT(vkCreateSemaphore(device->logicalDevice, &semaphoreCreateInfo, nullptr,
                                              &frameSync.renderComplete));

            VK_CHECK_RESULT(vkCreateFence(device->logicalDevice, &fenceCreateInfo, nullptr,
                                              &frameSync.inFlightSync));
        }

        currentFrame = 0;

        // Set up submit info structure
        // Semaphores will stay the same during application lifetime
        // Command buffer submission info is set by each example
        submitInfo = initializers::submitInfo();
        submitInfo.pWaitDstStageMask = &submitPipelineStages;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &framesSync[currentFrame].presentComplete;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &framesSync[currentFrame].renderComplete;

        swapChainFences.resize(swapChain.imageCount, VK_NULL_HANDLE);

    }

    void VulgineImpl::destroySyncPrimitives() {
        for(auto& frameSync: framesSync) {
            vkDestroySemaphore(device->logicalDevice, frameSync.presentComplete, nullptr);
            vkDestroySemaphore(device->logicalDevice, frameSync.renderComplete, nullptr);
            vkDestroyFence(device->logicalDevice, frameSync.inFlightSync, nullptr);
        }

        framesSync.clear();
    }

    void VulgineImpl::keyPressed(VulgineImpl::Window *window, int key) {
        if(!ImGui::GetIO().WantCaptureKeyboard)
            keyboardState.onKeyPressed(key);
        keyboardState.keysPressed.emplace(key, 0);
    }

    void VulgineImpl::mouseMoved(VulgineImpl::Window *window, double xPos, double yPos) {
        double dx = xPos - mouseState.cursor.posX;
        double dy = yPos - mouseState.cursor.posY;

        mouseState.cursor.posX = xPos;
        mouseState.cursor.posY = yPos;

        mouseState.onMouseMove(dx, dy, xPos, yPos);
    }

    void VulgineImpl::disableCursor() {
        glfwSetInputMode(window.instance(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        mouseState.cursor.enabled = false;
    }

    void VulgineImpl::enableCursor() {
        glfwSetInputMode(window.instance(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseState.cursor.enabled = true;

    }

    void VulgineImpl::setupDescriptorPools() {

        // TODO: determine number of sets and per-type distribution dynamically

        perMaterialPool.maxSets = 1024;

        std::map<VkDescriptorType, uint32_t> types;

        types[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 1 * 1024;
        types[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = 1 * 1024;

        perMaterialPool.descriptorsCapacity = std::move(types);


        std::map<VkDescriptorType, uint32_t> types1;

        types1[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 1 * 1024;
        types1[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER] = 1 * 1024;

        perMeshPool.maxSets = 1024;

        perMeshPool.descriptorsCapacity = std::move(types1);

        std::map<VkDescriptorType, uint32_t> types2;

        types2[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 1 * 1024;


        perScenePool.maxSets = 1024;

        perScenePool.descriptorsCapacity = std::move(types2);


    }

    void VulgineImpl::destroyDescriptorPools() {
        perMaterialPool.clear();
        perScenePool.clear();
        perMeshPool.clear();
    }

    Image *VulgineImpl::initNewImage() {
        return images.emplace();
    }

    void VulgineImpl::deleteImage(Image *image) {
        images.free(dynamic_cast<StaticImageImpl*>(image));
    }

    void VulgineImpl::updateGUI() {

        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2((float)vieportInfo.width, (float)vieportInfo.height);
        if(fpsCounter.lastFrameTime > 0.0)
            io.DeltaTime = fpsCounter.lastFrameTime;

        // If cursor is hidden, we tell ImGui not to track it

        if (!mouseState.cursor.enabled)
            io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        else
            io.MousePos = ImVec2(mouseState.cursor.posX, mouseState.cursor.posY);


        io.MouseDown[0] = mouseState.keys.left;
        io.MouseDown[1] = mouseState.keys.right;
        io.MouseDown[2] = mouseState.keys.middle;


        io.KeyCtrl = keyboardState.keysDown.count(GLFW_KEY_LEFT_CONTROL) ||
                     keyboardState.keysDown.count(GLFW_KEY_RIGHT_CONTROL);

        io.KeyShift = keyboardState.keysDown.count(GLFW_KEY_LEFT_SHIFT) ||
                      keyboardState.keysDown.count(GLFW_KEY_RIGHT_SHIFT);
        io.KeyAlt = keyboardState.keysDown.count(GLFW_KEY_LEFT_ALT) ||
                    keyboardState.keysDown.count(GLFW_KEY_RIGHT_ALT);

        memset(io.KeysDown, '\0', sizeof(decltype(io.KeysDown)));
        for (auto& key : keyboardState.keysPressed)
            io.KeysDown[key.first] = true;

        ImGui::NewFrame();

        // TODO: actual ui here

        guiImpl.draw();

        imgui.customGUI();

        ImGui::Render();

    }

    void VulgineImpl::mouseBtnDown(VulgineImpl::Window *window, int button) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: mouseState.keys.left = true; break;
            case GLFW_MOUSE_BUTTON_MIDDLE: mouseState.keys.middle = true; break;
            case GLFW_MOUSE_BUTTON_RIGHT: mouseState.keys.right = true; break;
            default: ;
        }

        mouseState.onMouseButtonDown(button);
    }

    void VulgineImpl::mouseBtnUp(VulgineImpl::Window *window, int button) {
        switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: mouseState.keys.left = false; break;
            case GLFW_MOUSE_BUTTON_MIDDLE: mouseState.keys.middle = false; break;
            case GLFW_MOUSE_BUTTON_RIGHT: mouseState.keys.right = false; break;
            default: ;
        }

        mouseState.onMouseButtonUp(button);
    }

    UniformBuffer *VulgineImpl::initNewUniformBuffer() {
        return uniformBuffers.emplace();
    }

    void VulgineImpl::deleteUniformBuffer(UniformBuffer *buffer) {
        uniformBuffers.free(dynamic_cast<UniformBufferImpl*>(buffer));
    }

    void VulgineImpl::mouseScroll(VulgineImpl::Window *window, double xoffset, double yoffset) {
        auto& io = ImGui::GetIO();

        io.MouseWheel = yoffset;
    }

    void VulgineImpl::charInput(VulgineImpl::Window *window, uint32_t unicode) {
        auto& io = ImGui::GetIO();

        io.AddInputCharacter(unicode);
    }

    void VulgineImpl::updateMSAA(VkSampleCountFlagBits newValue) {
        int maxMSAA = device->getMaximumMSAA();
        if(maxMSAA < newValue){
            errs("Cannot apply such high MSAA (device limit: " + std::to_string(maxMSAA) + ")");
            return;
        }

        vkQueueWaitIdle(queue);

        settings.msaa = newValue;


        vkDestroyImageView(device->logicalDevice, depthStencil.view, nullptr);
        vkDestroyImage(device->logicalDevice, depthStencil.image, nullptr);
        vkFreeMemory(device->logicalDevice, depthStencil.mem, nullptr);

        setupDepthStencil();

        buildRenderPasses();


    }

    void VulgineImpl::toggleVsync() {
        vkDeviceWaitIdle(device->logicalDevice);
        settings.vsync = !settings.vsync;
        setupSwapChain();
        destroyOnscreenFrameBuffers();
        createOnscreenFrameBuffers();
    }

    RenderPass *VulgineImpl::initNewRenderPass() {
        return renderPasses.emplace();
    }

    void VulgineImpl::deleteRenderPass(RenderPass *renderPass) {
        renderPasses.free(dynamic_cast<RenderPassImpl*>(renderPass));
    }

    void VulgineImpl::loadCustomShader(const char *filename, const char *name, ShaderStage stage) {
        auto shader = loadShader(filename, device->logicalDevice);
        auto* shaderMap = &fragmentShaders;

        switch (stage) {
            case ShaderStage::VERTEX:{
                shaderMap = &vertexShaders;
                break;
            }
            case ShaderStage::FRAGMENT:{
                shaderMap = &fragmentShaders;
                break;
            }
            default: assert(0 && "Invalid shader stage");

        }

        shaderMap->emplace(std::piecewise_construct, std::forward_as_tuple(name), std::forward_as_tuple(shader, name));

    }

    Sampler *VulgineImpl::initNewSampler() {
        return samplers.emplace();
    }

    void VulgineImpl::deleteSampler(Sampler *sampler) {
        samplers.free(dynamic_cast<SamplerImpl*>(sampler));
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


    void VulgineImpl::Window::init() {
        // creating window



        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


        instance_ = glfwCreateWindow(width, height,
                                           title.c_str(), nullptr, nullptr);


        if(fullscreen)
            goFullscreen();

        windowMap.emplace(instance_, this);


        monitorVideoModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &videoModeCount);

        logger("GLFW: Supported video modes:");



        for(int i = 0; i < videoModeCount; ++i){
            logger("r" + std::to_string(monitorVideoModes[i].redBits)
                 + "g" + std::to_string(monitorVideoModes[i].greenBits)
                 + "b" + std::to_string(monitorVideoModes[i].blueBits)
                 + " " + std::to_string(monitorVideoModes[i].width)
                 + "x" + std::to_string(monitorVideoModes[i].height)
                 + " " + std::to_string(monitorVideoModes[i].refreshRate)
                 + "hz");

            if(monitorVideoModes[i].redBits != 8 || monitorVideoModes[i].greenBits != 8 || monitorVideoModes[i].blueBits != 8)
                continue;
        }

        for(int i = videoModeCount - 1; i > -1; --i) {
            if(monitorVideoModes[i].redBits != 8 || monitorVideoModes[i].greenBits != 8 || monitorVideoModes[i].blueBits != 8)
                continue;
            selectedVideoMode = monitorVideoModes + i;
            break;
        }

        int width_in_pixels, height_in_pixels;

        glfwGetFramebufferSize(instance_, &width_in_pixels, &height_in_pixels);

        width = width_in_pixels;
        height = height_in_pixels;


        glfwGetWindowPos(instance_, &cachedWindowedDimensions.xPos, &cachedWindowedDimensions.yPos);
        glfwGetWindowSize(instance_, &cachedWindowedDimensions.width, &cachedWindowedDimensions.height);

        glfwSetFramebufferSizeCallback(instance_, windowSizeChanged);
        glfwSetKeyCallback(instance_, keyInput);
        glfwSetCursorPosCallback(instance_, cursorPosition);
        glfwSetMouseButtonCallback(instance_, mouseInput);
        glfwSetScrollCallback(instance_, scrollInput);
        glfwSetCharCallback(instance_, charInput);


    }

    void VulgineImpl::Window::terminate() {
        glfwDestroyWindow(instance_);
        windowMap.erase(instance_);
    }


    void VulgineImpl::Window::windowSizeChanged(GLFWwindow *window, int width, int height) {
        auto windowWrap = windowMap.find(window)->second;

        windowWrap->width = width;
        windowWrap->height = height;

        windowWrap->resized = true;

    }

    void VulgineImpl::Window::keyInput(GLFWwindow *window, int key, int scancode, int action, int mods) {

        auto* wrappedWindow = windowMap.at(window);

        switch(action){
            case GLFW_PRESS:{
                GetImpl().keyDown(wrappedWindow, key);
                break;
            }
            case GLFW_RELEASE:{
                GetImpl().keyUp(wrappedWindow, key);
                break;
            }
            case GLFW_REPEAT:{
                GetImpl().keyPressed(wrappedWindow, key);
                break;
            }
            default: break;
        }

    }

    void VulgineImpl::Window::goFullscreen() {
        glfwGetWindowPos(instance_, &cachedWindowedDimensions.xPos, &cachedWindowedDimensions.yPos);
        glfwGetWindowSize(instance_, &cachedWindowedDimensions.width, &cachedWindowedDimensions.height);

        if(selectedVideoMode != nullptr)
            glfwSetWindowMonitor(instance_, glfwGetPrimaryMonitor(), 0, 0, selectedVideoMode->width, selectedVideoMode->height, selectedVideoMode->refreshRate);
        else{
            glfwSetWindowMonitor(instance_, glfwGetPrimaryMonitor(), 0, 0, 1920, 1080, GLFW_DONT_CARE);
        }
        resized = true;
        fullscreen = true;
    }

    void VulgineImpl::Window::goWindowed() {

        int refreshRate = selectedVideoMode ? selectedVideoMode->refreshRate : GLFW_DONT_CARE;
        glfwSetWindowMonitor(instance_, nullptr,
                             cachedWindowedDimensions.xPos,
                             cachedWindowedDimensions.yPos,
                             cachedWindowedDimensions.width,
                             cachedWindowedDimensions.height,
                             refreshRate);

        resized = true;
        fullscreen = false;
    }

    void VulgineImpl::Window::setWindowTitle(std::string const& ttl) {
        title = ttl;

        glfwSetWindowTitle(instance_, title.c_str());
    }

    void VulgineImpl::Window::cursorPosition(GLFWwindow* window, double xPos, double yPos) {
        auto* wrappedWindow = windowMap.at(window);
        GetImpl().mouseMoved(wrappedWindow, xPos, yPos);
    }

    void VulgineImpl::Window::mouseInput(GLFWwindow *window, int button, int action, int mods) {
        auto* wrappedWindow = windowMap.at(window);
        if(action == GLFW_PRESS){
            GetImpl().mouseBtnDown(wrappedWindow, button);
        } else {
            GetImpl().mouseBtnUp(wrappedWindow, button);
        }


    }

    void VulgineImpl::Window::scrollInput(GLFWwindow *window, double xoffset, double yoffset) {
        auto* wrappedWindow = windowMap.at(window);
        GetImpl().mouseScroll(wrappedWindow, xoffset, yoffset);
    }

    void VulgineImpl::Window::charInput(GLFWwindow *window, uint32_t unicode) {
        auto* wrappedWindow = windowMap.at(window);
        GetImpl().charInput(wrappedWindow, unicode);
    }

    void VulgineImpl::FpsCounter::update(double deltaT) {

        lastFrameTime = deltaT;

        framesSinceLastTimeStamp++;
        timeSinceLastTimeStamp += deltaT;

        if(timeSinceLastTimeStamp >= period){
            fps = static_cast<double>(framesSinceLastTimeStamp) / timeSinceLastTimeStamp;
            timeSinceLastTimeStamp = 0.0;
            framesSinceLastTimeStamp = 0;
        }
    }

    void VulgineImpl::PipelineMap::add(PipelineKey key) {

        auto it = map.emplace(std::make_pair(key, key));

        if(it.second){
            it.first->second.create();
        }

    }

    Pipeline const& VulgineImpl::PipelineMap::bind(PipelineKey key, VkCommandBuffer cmdBuffer) {
        if(!map.count(key)){
            add(key);
        }
        Pipeline& pipeline = map.find(key)->second;

        pipeline.bind(cmdBuffer);

        return pipeline;
    }

    void VulgineImpl::PipelineMap::clear() {
        map.clear();
    }

    void Vulgine::MouseState::disableCursor() {
        GetImpl().disableCursor();
    }

    void Vulgine::MouseState::enableCursor() {
        GetImpl().enableCursor();
    }

}
