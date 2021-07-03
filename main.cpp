//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/IVulgine.h"



int main(int argc, char** argv){

    Vulgine::initializers.windowName = "HELLO THERE";
    Vulgine::initializers.windowSize = {1200, 800};

    auto* vulgine = Vulgine::Vulgine::createInstance();

    while(vulgine->cycle());

    Vulgine::Vulgine::freeInstance(vulgine);

    return 0;
}
