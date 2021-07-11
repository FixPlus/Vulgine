//
// Created by Бушев Дмитрий on 11.07.2021.
//

#ifndef TEST_EXE_VULKANSAMPLER_H
#define TEST_EXE_VULKANSAMPLER_H

#include <vulkan/vulkan.h>

namespace Vulgine{

    struct Sampler{
        VkSampler sampler;


        void create();
        void destroy();
    };
}
#endif //TEST_EXE_VULKANSAMPLER_H
