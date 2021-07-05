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
#include "IVulgineScene.h"

#define VULGINE_VERSION_MAJOR    0
#define VULGINE_VERSION_MINOR    0
#define VULGINE_VERSION_REVISION 0


namespace Vulgine {

    struct Initializers{
        std::optional<std::string> windowName;
        std::optional<std::pair<uint32_t, uint32_t>> windowSize;
        bool enableVulkanValidationLayers = false;
        std::string applicationName = "VulGineApp";

        void reset();
    };

    extern Initializers initializeInfo;

    struct Vulgine{
    protected:
        explicit Vulgine() = default;
        virtual ~Vulgine() = default;
    public:

        virtual Scene* initNewScene() = 0;
        virtual void deleteScene(Scene* scene) = 0;

        virtual Material* initNewMaterial() = 0;
        virtual void deleteMaterial(Material* scene) = 0;

        virtual void updateRenderTaskQueue(std::vector<RenderTask> const& renderTaskQueue) = 0;

        virtual bool cycle() = 0;
        static Vulgine* createInstance();
        static void freeInstance(Vulgine* instance);
    };

    std::string getLastErrorLog();
    void disableLog();
    void enableLog();
    void redirectLogTo(std::ostream& output);



    /** @brief: puts information about library version into given locations.
     *  @note: It may differ from ones defined in macros if header version mismatches from compiled artifacts **/

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
