#include "Vulgine.h"

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

    Initializers initializers;

    VulgineImpl::VulgineImpl(){

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::cout << extensionCount << " extensions supported\n";

    }

    bool VulgineImpl::cycle() {
        if(glfwWindowShouldClose(window.instance))
            return false;
        glfwPollEvents();
        return true;
    }

    VulgineImpl::~VulgineImpl() {
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

    Vulgine* Vulgine::createInstance(){

        // if no specific log file is set, proceed using standard output file

        if(logger.hasNullLogFile())
            logger.changeLogFile(&std::cout);

        if(vlg_instance == nullptr){

            // check version compatibility

            int major, minor, revision;

            int h_major, h_minor, h_revision;

            getVersion(&major, &minor, &revision);
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

        return true;
    }

    void VulgineImpl::createVkInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "VulGine";
        appInfo.engineVersion = VK_MAKE_VERSION(VULGINE_VERSION_MAJOR, VULGINE_VERSION_MINOR, VULGINE_VERSION_REVISION);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // TODO: implement external extensions list queries

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance))
    }

    void VulgineImpl::initFields() {

        if(initializers.windowSize) {
            auto size = initializers.windowSize.value();
            window.height = size.second;
            window.width = size.first;
        }

        if(initializers.windowName){
            window.name = initializers.windowName.value();
        }

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
        windowName.reset();
        windowSize.reset();
    }

    std::string getStringVersion() {
        std::string ret;
        ret = "VulGine " + std::to_string(VULGINE_VERSION_MAJOR) + "." + std::to_string(VULGINE_VERSION_MINOR) + "." + std::to_string(VULGINE_VERSION_REVISION);
        return ret;
    }
}
