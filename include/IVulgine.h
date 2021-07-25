//
// Created by Бушев Дмитрий on 03.07.2021.
//

#ifndef TEST_EXE_IVULGINE_H
#define TEST_EXE_IVULGINE_H

/*
 *
 *                         VULGINE
 *
 *   This library wraps over low-level graphical APIs - Vulkan and GLFW
 *   providing easier high-level graphical managing while not fully
 *   encapsulating them process inside, which grants ability for better control
 *   when necessary.
 *
 *
 */


#include <string>
#include <map>
#include <functional>
#include "IVulgineScene.h"

#define VULGINE_VERSION_MAJOR    0
#define VULGINE_VERSION_MINOR    0
#define VULGINE_VERSION_REVISION 0


namespace Vulgine {

    enum class ShaderStage{ VERTEX, FRAGMENT};

    struct Initializers{
        std::string windowName = "VulGineApp";
        std::pair<uint32_t, uint32_t> windowSize = {800, 600};
        bool enableVulkanValidationLayers = false;
        bool fullscreen = false;
        bool vsync = true;
        std::string applicationName = "VulGineApp";

        void reset();
    };

    extern Initializers initializeInfo;

    /**
     * @brief General interface with VulGine instance.
     *
     * @details
     * Retrieve an instance of it by calling createInstance() static member function.
     * Free all resources created by the instance by passing it to freeInstance().
     * All subresources are allocated and freed by the instance and must not be
     * deleted or/and used after instance invalidation by user.
     *
     * */

    struct Vulgine{
    protected:
        explicit Vulgine() = default;
        virtual ~Vulgine() = default;
    public:

        /**
         * Self-contained actual keyboard state.
         * Keys are represented by GLFW key table codes (GLFW_KEY_*)
         *
         * @keysDown contains keyboard keys, that are currently in 'down' state
         * @keysPressed contains keyboard keys, that are currently pressed
         */

        struct {
            std::map<int, int> keysDown;                            /** Filled by Vulgine */
            std::map<int, int> keysPressed;                         /** Filled by Vulgine */
            std::function<void(int)> onKeyPressed = [](int key) {}; /** Optional user-defined callback function*/
            std::function<void(int)> onKeyDown = [](int key) {};    /** Optional user-defined callback function*/
            std::function<void(int)> onKeyUp = [](int key) {};      /** Optional user-defined callback function*/
        } keyboardState;

        /**
         *
         * Self-contained Mouse State
         *
         */

        struct MouseState{
            struct{
                bool left = false;
                bool right = false;
                bool middle = false;
            } keys; /** Filled by Vulgine. Do not modify */

            std::function<void(int)> onMouseButtonDown = [](int){};  /** Optional user-defined callback function*/
            std::function<void(int)> onMouseButtonUp = [](int){};   /** Optional user-defined callback function*/

            std::function<void(double, double, double, double)> onMouseMove =
                    [](double dx, double dy, double x, double y) {};  /** Optional user-defined callback function*/

            struct {
                double posX, posY;
                bool enabled = true;
            } cursor; /** Filled by Vulgine. Do not modify */

            void disableCursor(); /** Hide cursor. Handy for camera mouse control purposes*/
            void enableCursor();
        } mouseState;

        /**
         *
         * ImGui handle user-defined function. All ImGui::*() calls must be contained in the scope of this function
         *
         */

        struct{
            std::function<void(void)> customGUI = [](){};
        } imgui;

        /**
         *  All top-level Vulgine API objects are allocated and freed by following functions
         *  Do not free pointers passed by this functions yourself!
         *  Pointers are valid since passed initXX function call and are invalidated after deleteXX
         *  call or freeInstance function call(in last case all resources retrieved by initXX calls are invalidated
         *  at once). Do not de-reference and use any pointer after their invalidation.
         *
         *  Following objects might allocate and pass their own resources as well(e.g. Scene case allocate Light objects).
         *  Pointers to such objects obey the same rules and invalidate after owning resource being freed.
         */

        virtual SceneRef initNewScene() = 0;

        virtual MaterialRef initNewMaterial() = 0;

        virtual ImageRef initNewImage() = 0;

        virtual UniformBufferRef initNewUniformBuffer() = 0;

        virtual RenderPassRef initNewRenderPass() = 0;

        virtual SamplerRef initNewSampler() = 0;

        virtual GeometryRef initNewGeometry() = 0;

        virtual MeshRef initNewMesh() = 0;

        virtual void loadCustomShader(const char* filename, const char* name, ShaderStage stage) = 0;


        virtual void buildRenderPasses() = 0;


        /**
         *
         * Function executed right-before command buffers recording. All dynamic Vulgine resources must be
         * updated only within this function scope.
         * Updating dynamic Vulgine resources outside cycle() function may corrupt rendering process.
         *
         */

        std::function<void(void)> onCycle = [](){};

        /**
         *
         * Main render loop. General use pattern: while(VulGine->cycle());
         *
         * @return true if quit condition is satisfied, false otherwise.
         */
        virtual bool cycle() = 0;
        virtual double lastFrameTime() const = 0;
        friend bool Init();
        friend void Terminate();
    };

    bool Init();

    Vulgine* Get();

    void Terminate();

    std::string getLastErrorLog();
    void disableLog();
    void enableLog();
    void redirectLogTo(std::ostream& output);



    /** @brief puts information about library version into given locations.
     *  @note It may differ from ones defined in macros if header version mismatches from compiled artifacts **/

    void getVersion(int* v_major, int* v_minor, int* v_revision);

    void getHeaderVersion(int* v_major, int* v_minor, int* v_revision)
#ifndef VULGINE_DO_NOT_DEFINE_HEADER_VERSION
    {
        *v_major = VULGINE_VERSION_MAJOR;
        *v_minor = VULGINE_VERSION_MINOR;
        *v_revision = VULGINE_VERSION_REVISION;
    }
#else
;
#endif
    std::string getStringVersion();

}

#endif //TEST_EXE_IVULGINE_H
