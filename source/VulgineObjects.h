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
#include "VulgineDescriptorSet.h"

#include <map>

namespace Vulgine{


    struct RenderPass;


    struct CameraImpl;
    struct SceneImpl;

    class UniformBufferImpl: public UniformBuffer, public ObjectImplNoMove{
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

    class MeshImpl: public Mesh, public ObjectImplNoMove{

        static MeshImpl* highlighted;
    public:

        void highlight() {highlighted = this;};
        static void clearHighlight() { highlighted = nullptr;}

        std::optional<DescriptorSet> set;


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

        explicit MeshImpl(uint32_t id): ObjectImplNoMove(Type::MESH, id){ };

        void createImpl() override;
        void destroyImpl() override;

        void pushVertexBuffer(int id);
        void pushInstanceBuffer(int id);

        void updateVertexBuffer() override;

        void updateIndexBuffer() override;

        void updateInstanceBuffer() override;

        void draw(VkCommandBuffer commandBuffer, SceneImpl* scene, CameraImpl *camera, RenderPass* pass, int currentFrame);


        ~MeshImpl() override;
    };

    struct LightImpl: public Light, public ObjectImplNoMove{

        explicit LightImpl(Scene* parent, uint32_t id): Light(parent), ObjectImplNoMove(Type::LIGHT, id){};

        void update() override;

        void createImpl() override;
        void destroyImpl() override;
        ~LightImpl() override{ /*TODO: disconnect from parent */};
    };

    struct ShaderModule: public ObjectImpl{
        VkShaderModule module;
        std::string name;

        explicit ShaderModule(VkShaderModule mod = VK_NULL_HANDLE, std::string nm = ""): ObjectImpl(Object::Type::SHADER_MODULE, ObjectImpl::claimId()), module(mod), name(std::move(nm)){}

        void createImpl() override;
        void destroyImpl() override;

        ~ShaderModule() override;
    };

    struct MaterialImpl: public Material, public ObjectImplNoMove{
        DescriptorSet set;
        SharedRef<UniformBufferImpl> ubo = nullptr;

        void update() override;

        explicit MaterialImpl(uint32_t id): ObjectImplNoMove(Type::MATERIAL, id){}
        void createImpl() override;
        void destroyImpl() override;

        ~MaterialImpl() override;
    };

    struct CameraImpl: public Camera, public ObjectImplNoMove{

        struct {
            glm::mat4 viewMatrix{};
            glm::vec3 position{};
        }matrices;

        void update() override;

        CameraImpl(Scene* parent, uint32_t id): Camera(parent), ObjectImplNoMove(Type::CAMERA, id){}
        void createImpl() override;
        void destroyImpl() override;

        ~CameraImpl() override{}
    };

    struct SamplerImpl: public Sampler, public ObjectImplNoMove{
        VkSampler sampler = VK_NULL_HANDLE;
        explicit SamplerImpl(uint32_t id): ObjectImplNoMove(Object::Type::SAMPLER, id){}

        ~SamplerImpl() override;
    protected:
        void createImpl() override;
        void destroyImpl() override;
    };

    struct GeometryImpl: public Geometry, public ObjectImplNoMove{
        VkVertexInputBindingDescription vertexAttrBindingInfo[2] = {};

        std::vector<VkVertexInputAttributeDescription> attributesDesc{};

        void compileVertexInputState();
    public:
        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};

        explicit GeometryImpl(uint32_t id): ObjectImplNoMove(Type::GEOMETRY, id) {}

        ~GeometryImpl() override;
    protected:
        void createImpl() override;
        void destroyImpl() override;
    };
}
#endif //TEST_EXE_VULGINEOBJECTS_H
