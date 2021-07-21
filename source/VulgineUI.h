//
// Created by Бушев Дмитрий on 16.07.2021.
//

#ifndef TEST_EXE_VULGINEUI_H
#define TEST_EXE_VULGINEUI_H

#include "imgui/imgui.h"
#include "IVulgineObjects.h"
#include <vector>

namespace Vulgine{


    class ObjectImpl;

    class MaterialImpl;
    class SceneImpl;
    class StaticImageImpl;
    class UniformBufferImpl;

    class ContentWindow{
    protected:
        bool opened = false;
    public:

        virtual void draw() = 0;

        void open() { opened = true;};
        void close() { opened = false;};

        bool isOpened() const { return opened;};

        virtual ~ContentWindow() = default;
    };

    class ObjectInspector: public ContentWindow{

        ObjectImpl* selectedObject;

        void displaySceneInfo();
        void displayMaterialInfo();
        void displayImageInfo();
        void displayUBOInfo();
        void displayMeshInfo();
        void displayRenderPassInfo();
        void displayFrameBufferInfo();
        void displayCameraInfo();
        void displayLightInfo();

        void select(ObjectImpl* object);


        void selectable(ObjectImpl* object);

    public:

        void draw() override;

    };

    class UserInterface {
        static ImGuiTextBuffer logBuf;

        ObjectInspector* pObjectInspector = nullptr;
        std::vector<ContentWindow*> windows;

    public:

        UserInterface();

        static void addLog(const char* log, ...);

        bool opened = false;

        bool metricsViewerOpened = false;
        bool systemPropertiesOpened = false;
        bool logOpened = false;
        bool aboutOpened = false;
        void drawLogWindow();
        void drawMetricsViewerWindow();
        void drawSystemPropertiesWindow();
        void drawAboutWindow();
        void draw();

        ~UserInterface();
    };


}
#endif //TEST_EXE_VULGINEUI_H
