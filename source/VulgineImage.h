//
// Created by Бушев Дмитрий on 11.07.2021.
//

#ifndef TEST_EXE_VULGINEIMAGE_H
#define TEST_EXE_VULGINEIMAGE_H

#include "VulgineObject.h"
#include "vulkan/VulkanAllocatable.h"


namespace Vulgine{

    struct StaticImageImpl: public Image, public ObjectImplNoMove{
        Memory::Image image;

        explicit StaticImageImpl(uint32_t id): ObjectImplNoMove(Type::IMAGE, id){}
        bool loadFromPixelData(const unsigned char* data, int texWidth, int texHeight, FileFormat fileFormat);
        bool loadFromFile(const char* filename, FileFormat fileFormat) override;
        bool load(const unsigned char* data, uint32_t len, FileFormat fileFormat) override;

        ~StaticImageImpl() override;

    protected:
        void createImpl() override;
        void destroyImpl() override;
    };

    struct DynamicImageImpl: public Image, public ObjectImplNoMove{
        std::vector<Memory::Image> images;
        VkImageCreateInfo createInfo{};

        explicit DynamicImageImpl(uint32_t id): ObjectImplNoMove(Type::IMAGE, id){}

        // TODO: I should reconsider following functions being part of Image general interface because for dynamic images they make no sense

        bool loadFromFile(const char* filename, FileFormat fileFormat) override{ return false;};
        bool load(const unsigned char* data, uint32_t len, FileFormat fileFormat) override{ return false;};

    protected:
        void createImpl() override;
        void destroyImpl() override;
    };
}
#endif //TEST_EXE_VULGINEIMAGE_H
