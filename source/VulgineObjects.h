//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINEOBJECTS_H
#define TEST_EXE_VULGINEOBJECTS_H

#include <VulgineObject.h>
#include <../include/IVulgineScene.h>
#include "Utilities.h"
#include "vulkan/VulkanAllocatable.h"
#include "vulkan/VulkanDescriptable.h"
#include <map>

namespace Vulgine{


    struct RenderPass;


    struct CameraImpl;

    class UniformBufferImpl: public UniformBuffer, public ObjectImpl{
    public:
        std::vector<Memory::Buffer*> buffers{};
        std::vector<bool> updated{};
        explicit UniformBufferImpl(uint32_t id);

        void update() override;
        void sync();

        void createImpl() override;
        void destroyImpl() override;

        ~UniformBufferImpl() override;
    };

    class MeshImpl: public Mesh, public ObjectImpl{
        static uint32_t count_;
        VkVertexInputBindingDescription vertexAttrBindingInfo[2] = {};

        std::vector<VkVertexInputAttributeDescription> attributesDesc{};

        void compileVertexInputState();

    public:

        std::vector<uint32_t> descriptorSets;

        std::vector<std::vector<Descriptable*>> descriptors;

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};

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

        explicit MeshImpl(Scene* parent, uint32_t id): Mesh(parent), ObjectImpl(id, Type::MESH){ count_++; };

        void createImpl() override;
        void destroyImpl() override;

        void pushVertexBuffer(int id);
        void pushInstanceBuffer(int id);

        void updateVertexBuffer() override;

        void updateIndexBuffer() override;

        void updateInstanceBuffer() override;

        void draw(VkCommandBuffer commandBuffer, CameraImpl *camera, RenderPass* pass);

        static uint32_t count() {return count_;};

        ~MeshImpl() override;
    };

    struct LightImpl: public Light, public ObjectImpl{

        explicit LightImpl(Scene* parent, uint32_t id): Light(parent), ObjectImpl(id, Type::LIGHT){};


        void createImpl() override;
        void destroyImpl() override;
        ~LightImpl() override{ /*TODO: disconnect from parent */};
    };

    struct ShaderModule: public ObjectImpl{
        VkShaderModule module;
        std::string name;

        explicit ShaderModule(VkShaderModule mod = VK_NULL_HANDLE, std::string nm = ""): ObjectImpl(0, Object::Type::SHADER_MODULE), module(mod), name(std::move(nm)){}

        void createImpl() override;
        void destroyImpl() override;

        ~ShaderModule() override;
    };

    struct MaterialImpl: public Material, public ObjectImpl{
        uint32_t descriptorSet;
        bool hasDescriptorSet = false;
        struct TextureSampler{
            VkDescriptorImageInfo descriptor;
            Sampler sampler;
        } colorMapSampled, normalMapSampled;



        explicit MaterialImpl(uint32_t id): ObjectImpl(id, Type::MATERIAL){}
        void createImpl() override;
        void destroyImpl() override;

        ~MaterialImpl() override;
    };

    struct CameraImpl: public Camera, public ObjectImpl{

        struct {
            glm::mat4 viewMatrix{};
            glm::vec3 position{};
        }matrices;

        void update() override;

        CameraImpl(Scene* parent, uint32_t id): Camera(parent), ObjectImpl(id, Type::CAMERA){}
        void createImpl() override;
        void destroyImpl() override;

        ~CameraImpl() override{}
    };

}
#endif //TEST_EXE_VULGINEOBJECTS_H
