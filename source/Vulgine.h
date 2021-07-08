#ifndef VULGINE_VULGINE_H
#define VULGINE_VULGINE_H

#define VULGINE_DO_NOT_DEFINE_HEADER_VERSION
#include "IVulgine.h"
#undef VULGINE_DO_NOT_DEFINE_HEADER_VERSION

#include "vulkan/VulkanDevice.h"
#include "vulkan/VulkanSwapChain.h"
#include "vulkan/VulkanAllocatable.h"
#include "VulgineRenderPass.h"
#include "VulginePipeline.h"
#include <vector>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulgine {


    class VulgineImpl: public Vulgine{
        static std::vector<const char*> getRequiredExtensionsList();

        VkBool32 getSupportedDepthFormat();
    protected:
        class Window : public Creatable {
            static std::map<GLFWwindow *, Window*> windowMap;

            // callbacks

            static void windowSizeChanged(GLFWwindow* window, int width, int height);
            static void keyInput(GLFWwindow* window, int key, int scancode, int action, int mods);

            GLFWwindow *instance_ = nullptr;

            void createImpl() override;
            void destroyImpl() override;

            struct{
                int xPos, yPos, width, height;
            } cachedWindowedDimensions;

        public:

            const GLFWvidmode* monitorVideoModes = nullptr;
            const GLFWvidmode* selectedVideoMode = nullptr;
            int videoModeCount = 0;

            std::string name = "VulGine App";
            std::string title = "VulGine App";
            uint32_t width = 800;
            uint32_t height = 600;

            bool fullscreen = false;

            bool resized = false;


            [[nodiscard]] GLFWwindow* instance() const { return instance_;};

            void goFullscreen();
            void goWindowed();

            void setWindowTitle(std::string const& title);

            ~Window() override;
        } window;

        struct FpsCounter{
            int framesSinceLastTimeStamp = 0;
            double timeSinceLastTimeStamp = 0.0f;
        public:
            double fps = 0;
            double period = 1.0f; //seconds
            double lastFrameTime = 0.0f;
            void update(double deltaT);
        } fpsCounter;
        // contains actual viewport dimensions (may differ from window dimensions if window size has just changed)

        struct{
            std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> tStart;
            std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> tEnd;
        } timeMarkers;
        struct{
            uint32_t width;
            uint32_t height;
        } vieportInfo;

        VkInstance instance = VK_NULL_HANDLE;


        /** @brief Set of physical device features to be enabled for this example (must be set in the derived constructor) */
        VkPhysicalDeviceFeatures enabledFeatures{};
        /** @brief Set of device extensions to be enabled for this example (must be set in the derived constructor) */
        std::vector<const char*> enabledDeviceExtensions;

        /** @brief Optional pNext structure for passing extension structures to device creation */
        void* deviceCreatepNextChain = nullptr;

        VulkanSwapChain swapChain;

        bool prepared = false;

        // Command buffer pool
        VkCommandPool cmdPool;
        // Command buffers used for rendering
        std::vector<VkCommandBuffer> drawCmdBuffers;
        // Active frame buffer index
        uint32_t currentBuffer = 0;

        // Onscreen frame buffers (one per each swap chain image)

        std::vector<Framebuffer> onScreenFramebuffers;

        // Offscreen frame buffers (one per each offscreen render pass)

        std::vector<Framebuffer> offScreenFramebuffers;

        // depth/stencil image used by onscreen renderPass (only one for all frame buffers)

        struct {
            VkImage image;
            VkDeviceMemory mem;
            VkImageView view;
        } depthStencil;

        std::vector<RenderTask> taskQueue;
        // Container for scene objects

        struct {
            std::stack<uint32_t> freeIds;
            std::unordered_map<uint32_t, SceneImpl> container;
        } scenes;

        // Container for materials

        struct {
            std::stack<uint32_t> freeIds;
            std::unordered_map<uint32_t, MaterialImpl> container;
        } materials;


        // Depth buffer format (selected during Vulkan initialization)
        VkFormat depthFormat;

        void createVkInstance();
        void createVulkanDevice();
        void createCommandBuffers();
        void setupDepthStencil();
        void setupSwapChain();
        void createCommandPool();
        void createPipelineCache();

        void destroyRenderPasses();
        void destroyCommandBuffers();

        void initFields();
        void renderFrame();

        void buildRenderPasses();
        void buildCommandBuffers();
        void createOnscreenFrameBuffers();
        void destroyOnscreenFrameBuffers();

        void loadShaders();
        void destroyShaders();

        void windowResize();


    public:

        struct Settings{
            bool vsync;
        } settings;

        // User input functions called by active window input listener functions

        void keyDown(Window* window, int key);
        void keyUp(Window* window, int key);


        std::map<std::string, ShaderModule> vertexShaders;
        std::map<std::string, ShaderModule> fragmentShaders;

        // Container for pipelines

        std::map<std::tuple<MaterialImpl*, SceneImpl*, RenderPass*>, Pipeline> pipelines;


        VulkanDevice* device = nullptr;
        VmaAllocator allocator = VK_NULL_HANDLE;

        VkQueue queue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;

        // Pipeline cache object
        VkPipelineCache pipelineCache;

        // Queue of render passes. Must contain at least 1 pass (onscreen)

        std::vector<RenderPass*> renderPasses;

        RenderPass* onscreenRenderPass;


        bool cmdBuffersOutdated = false;

        void updateRenderTaskQueue(std::vector<RenderTask> const& renderTaskQueue) override;

        Scene* initNewScene() override;
        void deleteScene(Scene* scene) override;

        Material* initNewMaterial() override;
        void deleteMaterial(Material* scene) override;
        bool initialize();
        explicit VulgineImpl();
        ~VulgineImpl() override;
        bool cycle() override;
        double lastFrameTime() const override;
    };
    extern VulgineImpl* vlg_instance;



}

#endif //VULGINE_VULGINE_H
