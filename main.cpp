//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/Vulgine.h"



int main(int argc, char** argv){

    auto* vulgine = Vulgine::Vulgine::createInstance();

    while(vulgine->cycle());

    Vulgine::Vulgine::freeInstance(vulgine);

    return 0;
}
