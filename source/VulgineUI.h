//
// Created by Бушев Дмитрий on 16.07.2021.
//

#ifndef TEST_EXE_VULGINEUI_H
#define TEST_EXE_VULGINEUI_H

#include "imgui/imgui.h"
#include <vector>

namespace Vulgine{

    class MaterialImpl;
    class SceneImpl;
    class ImageImpl;
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
        enum class ObjectType{
            MATERIAL,
            SCENE,
            IMAGE,
            UBO,
            NONE
        } selectedType = ObjectType::NONE;

        union SelectedObject {
            MaterialImpl *material;
            SceneImpl *scene;
            ImageImpl *image;
            UniformBufferImpl *ubo;
        } selectedObject;

        void displaySceneInfo();
        void displayMaterialInfo();
        void displayImageInfo();
        void displayUBOInfo();

        void select(MaterialImpl* material);
        void select(SceneImpl* scene);
        void select(ImageImpl* image);
        void select(UniformBufferImpl* ubo);

        void selectable(MaterialImpl* material);
        void selectable(SceneImpl* scene);
        void selectable(ImageImpl* image);
        void selectable(UniformBufferImpl* ubo);
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
        void drawObjectInspectorWindow();
        void drawMetricsViewerWindow();
        void drawSystemPropertiesWindow();
        void drawAboutWindow();
        void draw();

        ~UserInterface();
    };


}
#endif //TEST_EXE_VULGINEUI_H
