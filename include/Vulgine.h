#ifndef VULGINE_VULGINE_H
#define VULGINE_VULGINE_H
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


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>
#include <string>
#include <map>
namespace Vulgine {

    struct PreSettings{
        std::optional<std::string> windowName;
        std::optional<glm::vec2> windowSize;
    };

    class Vulgine{

        struct Window {
            GLFWwindow *instance = nullptr;
            std::string name = "VulGine App";
            uint32_t width = 800;
            uint32_t height = 600;
        } window;

        VkInstance instance;

        void createVkInstance();
        explicit Vulgine();
        ~Vulgine();

        void setup(PreSettings const& settings);
    public:
        /** @brief: performs a single rendering cycle. returns false if quit condition was encountered **/
        bool cycle();
        static Vulgine* createInstance(PreSettings const& settings = {});
        static void freeInstance(Vulgine* instance);

    };

    std::string getLastErrorLog();
    void disableLog();
    void enableLog();
    void redirectLogTo(std::ostream& output);

}

#endif //VULGINE_VULGINE_H
