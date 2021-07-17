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
#include "vulkan/VulkanDescriptorPool.h"
#include "VulgineUI.h"
#include "VulgineImage.h"
#include <vector>
#include <chrono>
#include <vulkan/VulkanImGui.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulgine {

    template<typename T>
    class IdentifiableContainer{
        std::stack<uint32_t> freeIds;
        std::unordered_map<uint32_t, T> container;
    public:
        T* emplace(){
            uint32_t id;
            if(!freeIds.empty()) {
                id = freeIds.top();
                freeIds.pop();
            } else{
                id = container.size();
            }

            return &((container.emplace(std::piecewise_construct,std::forward_as_tuple(id), std::forward_as_tuple(id)).first)->second);
        };
        void free(T* obj){
            auto id = obj->id();

            if(id != container.size() - 1)
                freeIds.push(id);

            container.erase(id);
        }

        void iterate(std::function<void(T&)> operation){
            for(auto& obj: container)
                operation(obj.second);
        }

        void clear(){
            while(!freeIds.empty())
                freeIds.pop();

            container.clear();
        }

        size_t size() const{
            return container.size();
        }
    };


    class VulgineImpl: public Vulgine{
        static std::vector<const char*> getRequiredExtensionsList();

        VkBool32 getSupportedDepthFormat();
    protected:
        class Window : public ObjectImpl {
            static std::map<GLFWwindow *, Window*> windowMap;

            // callbacks

            static void windowSizeChanged(GLFWwindow* window, int width, int height);
            static void keyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
            static void cursorPosition(GLFWwindow* window, double xPos, double yPos);
            static void mouseInput(GLFWwindow* window, int button, int action, int mods);
            static void scrollInput(GLFWwindow* window, double xoffset, double yoffset);
            static void charInput(GLFWwindow* window, uint32_t unicode);

            GLFWwindow *instance_ = nullptr;

            void createImpl() override;
            void destroyImpl() override;

            struct{
                int xPos = 0, yPos = 0, width = 0, height = 0;
            } cachedWindowedDimensions;

        public:

            Window(): ObjectImpl(0, Object::Type::UNKNOWN){}

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


        struct{
            std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> tStart;
            std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> tEnd;
        } timeMarkers;

        // contains actual viewport dimensions (may differ from window dimensions if window size has just changed)

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



        bool prepared = false;

        /** @brief Pipeline stages used to wait at for graphics queue submissions */
        const VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        // Synchronization for queue operations

        struct FrameSyncObj {
            // Swap chain image presentation
            VkSemaphore presentComplete;
            // Command buffer submission and execution
            VkSemaphore renderComplete;

            VkFence inFlightSync;
        };

        std::vector<VkFence> swapChainFences;
        std::vector<FrameSyncObj> framesSync;

        UserInterface guiImpl;

        VkSubmitInfo submitInfo;

        // Command buffer pool
        VkCommandPool cmdPool;
        // Command buffers used for rendering
        std::vector<VkCommandBuffer> drawCmdBuffers;
        // Active frame buffer index
        uint32_t currentBuffer = 0;

        // Onscreen frame buffers (one per each swap chain image)

        std::vector<Framebuffer> onScreenFramebuffers;

        struct{
            Memory::Image image;
            VkImageView view;
        } msaaImage;

        // Offscreen frame buffers (one per each offscreen render pass)

        std::vector<Framebuffer> offScreenFramebuffers;

        // depth/stencil image used by onscreen renderPass (only one for all frame buffers)

        struct {
            VkImage image;
            VkDeviceMemory mem;
            VkImageView view;
        } depthStencil;

        std::vector<RenderTask> taskQueue;



        // Depth buffer format (selected during Vulkan initialization)
        VkFormat depthFormat;

        void createVkInstance();
        void createVulkanDevice();
        void createCommandBuffers();
        void createSyncPrimitives();
        void destroySyncPrimitives();
        void setupDepthStencil();
        void setupSwapChain();
        void setupDescriptorPools();
        void destroyDescriptorPools();
        void createCommandPool();
        void createPipelineCache();

        void destroyRenderPasses();
        void destroyCommandBuffers();

        void initFields();
        void renderFrame();

        void buildRenderPasses();
        void buildCommandBuffers(int imageIndex);
        void createOnscreenFrameBuffers();
        void destroyOnscreenFrameBuffers();

        void loadShaders();
        void destroyShaders();

        void windowResize();

        void updateGUI();

    public:

        struct Settings{
            bool vsync = false;
            VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;
            uint32_t framesInFlight = 2;
        } settings;

        // User input functions called by active window input listener functions

        void keyDown(Window* window, int key);
        void keyPressed(Window* window, int key);
        void keyUp(Window* window, int key);
        void mouseMoved(Window* window, double xPos, double yPos);
        void mouseBtnDown(Window* window, int button);
        void mouseBtnUp(Window* window, int button);
        void mouseScroll(Window* window, double xoffset, double yoffset);
        void charInput(Window* window, uint32_t unicode);

        void disableCursor();
        void enableCursor();

        std::map<std::string, ShaderModule> vertexShaders;
        std::map<std::string, ShaderModule> fragmentShaders;

        IdentifiableContainer<SceneImpl> scenes;
        IdentifiableContainer<MaterialImpl> materials;
        IdentifiableContainer<ImageImpl> images;
        IdentifiableContainer<UniformBufferImpl> uniformBuffers;

        DescriptorPool perScenePool;    // set 0
        DescriptorPool perMaterialPool; // set 1
        DescriptorPool perMeshPool;     // set 2

        // Container for pipelines

        struct PipelineMap{


            std::map<PipelineKey , Pipeline> map;

            void add(PipelineKey key);

            Pipeline const& bind(PipelineKey key, VkCommandBuffer cmdBuffer);

            void clear();

        } pipelineMap;

        VulkanSwapChain swapChain;

        int currentFrame;



        VulkanDevice* device = nullptr;
        VmaAllocator allocator = VK_NULL_HANDLE;

        VkQueue queue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;

        // Pipeline cache object
        VkPipelineCache pipelineCache;

        // Queue of render passes. Must contain at least 1 pass (onscreen)

        std::vector<RenderPass*> renderPasses;

        RenderPass* onscreenRenderPass;

        GUI gui;

        bool cmdBuffersOutdated = false;

        void updateRenderTaskQueue(std::vector<RenderTask> const& renderTaskQueue) override;

        Scene* initNewScene() override;
        void deleteScene(Scene* scene) override;

        Material* initNewMaterial() override;
        void deleteMaterial(Material* scene) override;

        Image* initNewImage() override;
        void deleteImage(Image* image) override;

        UniformBuffer* initNewUniformBuffer() override;
        void deleteUniformBuffer(UniformBuffer* buffer) override;

        bool initialize();
        explicit VulgineImpl();
        ~VulgineImpl() override;
        bool cycle() override;
        double lastFrameTime() const override;
        void updateMSAA(VkSampleCountFlagBits newValue);
        void toggleVsync();

        friend class UserInterface;
    };
    extern VulgineImpl* vlg_instance;



}

#endif //VULGINE_VULGINE_H
