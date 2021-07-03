#ifndef VULGINE_VULGINE_H
#define VULGINE_VULGINE_H
#include "../include/IVulgine.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Vulgine {


    class VulgineImpl: public Vulgine{
    protected:
        struct Window {
            GLFWwindow *instance = nullptr;
            std::string name = "VulGine App";
            uint32_t width = 800;
            uint32_t height = 600;
        } window;

        VkInstance instance;

        void createVkInstance();

        void initFields();
    public:
        bool initialize();
        explicit VulgineImpl();
        ~VulgineImpl();
        bool cycle() override;

    };


}

#endif //VULGINE_VULGINE_H
