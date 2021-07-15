//
// Created by Бушев Дмитрий on 16.07.2021.
//

#ifndef TEST_EXE_VULGINEUI_H
#define TEST_EXE_VULGINEUI_H

namespace Vulgine{



    class UserInterface{
    public:
        bool opened = false;

        bool objectInspectorOpened = false;
        bool metricsViewerOpened = false;
        bool systemPropertiesOpened = false;
        bool aboutOpened = false;
        void drawObjectInspectorWindow();
        void drawMetricsViewerWindow();
        void drawSystemPropertiesWindow();
        void drawAboutWindow();
        void draw();
    };


}
#endif //TEST_EXE_VULGINEUI_H
