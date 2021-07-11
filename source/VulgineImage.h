//
// Created by Бушев Дмитрий on 11.07.2021.
//

#ifndef TEST_EXE_VULGINEIMAGE_H
#define TEST_EXE_VULGINEIMAGE_H

#include "IVulgineObjects.h"
#include "vulkan/VulkanAllocatable.h"

namespace Vulgine{

    struct ImageImpl: public Image{
        Memory::Image image;

        explicit ImageImpl(uint32_t id): Image(id){}
        void loadFromPixelData(const unsigned char* data, int texWidth, int texHeight, FileFormat fileFormat);
        void loadFromFile(const char* filename, FileFormat fileFormat) override;
        void load(const unsigned char* data, uint32_t len, FileFormat fileFormat) override;

        ~ImageImpl() override;
    };
}
#endif //TEST_EXE_VULGINEIMAGE_H
