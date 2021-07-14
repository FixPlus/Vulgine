//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINEOBJECTS_H
#define TEST_EXE_VULGINEOBJECTS_H

#include <../include/IVulgineObjects.h>
#include <../include/IVulgineScene.h>
#include "Utilities.h"
#include "vulkan/VulkanAllocatable.h"
#include "vulkan/VulkanDescriptable.h"
#include <map>

namespace Vulgine{

    struct RenderPass;


    struct CameraImpl;

    class MeshImpl: public Mesh{
        VkVertexInputBindingDescription vertexAttrBindingInfo[2] = {};

        std::vector<VkVertexInputAttributeDescription> attributesDesc{};

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};

        void compileVertexInputState();

    public:
        struct{
            void* pData = nullptr;
            uint32_t count = 0;
        } cachedVertices;

        struct{
            void* pData = nullptr;
            uint32_t count = 0;
        } cachedInstances;

        std::vector<std::pair<Memory::VertexBuffer*, bool>> perVertex;
        std::vector<std::pair<Memory::VertexBuffer*, bool>> perInstance;

        Memory::StaticIndexBuffer indexBuffer;

        explicit MeshImpl(Scene* parent, uint32_t id): Mesh(parent, id){ logger("Mesh created");};

        void createImpl() override;
        void destroyImpl() override;

        void pushVertexBuffer(int id);
        void pushInstanceBuffer(int id);

        void updateVertexBuffer() override;

        void updateIndexBuffer() override;

        void updateInstanceBuffer() override;

        void draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass* pass);


        ~MeshImpl() override;
    };

    struct LightImpl: public Light{

        explicit LightImpl(Scene* parent, uint32_t id): Light(parent, id){};


        void createImpl() override;
        void destroyImpl() override;
        ~LightImpl() override{ /*TODO: disconnect from parent */};
    };

    struct ShaderModule: public Creatable{
        VkShaderModule module = VK_NULL_HANDLE;
        std::string name = "";

        ShaderModule() = default;
        ShaderModule(VkShaderModule mod, std::string nm): module(mod), name(std::move(nm)){}

        void createImpl() override;
        void destroyImpl() override;

        ~ShaderModule() override;
    };

    struct MaterialImpl: public Material{
        uint32_t descriptorSet;
        bool hasDescriptorSet = false;
        struct TextureSampler{
            VkDescriptorImageInfo descriptor;
            Sampler sampler;
        } colorMapSampled, normalMapSampled;



        explicit MaterialImpl(uint32_t id): Material(id){}
        void createImpl() override;
        void destroyImpl() override;

        ~MaterialImpl() override;
    };

    struct CameraImpl: public Camera{

        struct {
            glm::mat4 viewMatrix{};
            glm::vec3 position{};
        }matrices;

        void update() override;

        CameraImpl(Scene* parent, uint32_t id): Camera(parent, id){logger("Camera created");}
        void createImpl() override;
        void destroyImpl() override;

        ~CameraImpl() override{ logger("Camera freed");}
    };

}
#endif //TEST_EXE_VULGINEOBJECTS_H
