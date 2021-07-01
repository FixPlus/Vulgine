//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/Vulgine.h"



int main(int argc, char** argv){

    Vulgine::PreSettings settings;

    settings.windowName = "HELLO THERE";
    settings.windowSize = glm::vec2{1200, 800};

    auto* vulgine = Vulgine::Vulgine::createInstance(settings);

    while(vulgine->cycle());

    Vulgine::Vulgine::freeInstance(vulgine);

    return 0;
}
