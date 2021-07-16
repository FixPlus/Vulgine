//
// Created by Бушев Дмитрий on 16.07.2021.
//

#ifndef TEST_EXE_VULGINEUI_H
#define TEST_EXE_VULGINEUI_H

#include "imgui/imgui.h"

namespace Vulgine{



    class UserInterface{
        static ImGuiTextBuffer logBuf;
    public:


        static void addLog(const char* log, ...);

        bool opened = false;

        bool objectInspectorOpened = false;
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
    };


}
#endif //TEST_EXE_VULGINEUI_H
