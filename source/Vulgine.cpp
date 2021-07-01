#include "../include/Vulgine.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <string>
#include <sstream>
#include <Utilities.h>
#include <vector>

namespace Vulgine{

    Vulgine* vlg_instance = nullptr;
    std::string err_message = "";

    Vulgine::Vulgine(){

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::cout << extensionCount << " extensions supported\n";

    }

    bool Vulgine::cycle() {
        if(glfwWindowShouldClose(window.instance))
            return false;
        glfwPollEvents();
        return true;
    }

    Vulgine::~Vulgine() {
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window.instance);
        glfwTerminate();
    }

    std::string getLastErrorLog(){
        return err_message;
    }

    void error_callback(int code, const char* description){
        errs("GLFW error: " + std::string(description) + "(Error code: " + std::to_string(code) + ")");
    }

    Vulgine* Vulgine::createInstance(PreSettings const& settings){

        // if no specific log file is set, proceed using standard output file

        if(logger.hasNullLogFile())
            logger.changeLogFile(&std::cout);

        if(vlg_instance == nullptr){

            try{
                vlg_instance = new Vulgine();
            }
            catch (std::bad_alloc const& e){
                vlg_instance = nullptr;
                errs("Can't create VulGine instance: out of RAM");
                return nullptr;
            }
            logger("VulGine Instance allocated");

            vlg_instance->setup(settings);

            // checking if loaded GLFW library has compatible version

            int major, minor, revision;
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
            // initializing GLFW

            glfwInit();

            int code = glfwGetError(nullptr);

            if (code != GLFW_NO_ERROR){
                errs("GLFW Initialization failed with following error code: " + std::to_string(code));
                return nullptr;
            }
            logger("GLFW Initialized");

            glfwSetErrorCallback(error_callback);


            // creating window

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            auto window = glfwCreateWindow(vlg_instance->window.width, vlg_instance->window.height,
                                           vlg_instance->window.name.c_str(), nullptr, nullptr);

            vlg_instance->window.instance = window;

            logger("GLFW: created window");


            //TODO: vulkan instance, physical and virtual devices initialization processes

            vlg_instance->createVkInstance();

            logger("Vulkan Instance created");

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

    void Vulgine::setup(const PreSettings &settings) {
        if(settings.windowSize) {
            auto size = settings.windowSize.value();
            window.height = size.y;
            window.width = size.x;
        }

        if(settings.windowName){
            window.name = settings.windowName.value();
        }
    }

    void Vulgine::createVkInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VulGine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance))
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


}
