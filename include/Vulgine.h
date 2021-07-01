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
#include <string>

namespace Vulgine {

    class Vulgine{
        GLFWwindow* window = nullptr;
        explicit Vulgine();
        ~Vulgine();
    public:
        /** @brief: performs a single rendering cycle. returns false if quit condition was encountered **/
        bool cycle();
        static Vulgine* createInstance();
        static void freeInstance(Vulgine* instance);

    };

    std::string getLastErrorLog();
    void disableLog();
    void enableLog();
    void redirectLogTo(std::ostream& output);

}

#endif //VULGINE_VULGINE_H
