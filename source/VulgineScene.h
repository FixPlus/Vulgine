//
// Created by Бушев Дмитрий on 04.07.2021.
//

#ifndef TEST_EXE_VULGINESCENE_H
#define TEST_EXE_VULGINESCENE_H

#include <../include/IVulgineScene.h>
#include <VulgineObjects.h>

#include <unordered_map>

namespace Vulgine{

    constexpr const uint32_t MAX_LIGHTS = 20;

    struct RenderPass;

    struct SceneImpl: public Scene, public ObjectImplNoMove{

        struct{
            struct{
                glm::vec4 lightColor;
                glm::vec4 lightDirection;
            } lights[MAX_LIGHTS] = {};
            uint32_t lightCount = 0;
        } lightsInfo;

        struct{
            bool created = false;
            MaterialImpl material{claimId()};
        } background;

        UniformBufferImpl lightUBO;


        explicit SceneImpl(uint32_t id): lightUBO(ObjectImpl::claimId()), ObjectImplNoMove(Type::SCENE, id){};

        std::unordered_map<uint32_t, uint32_t> lightMap;
        std::unordered_map<uint32_t, uint32_t> reverseLightMap;

        std::unordered_map<uint32_t, MeshImpl> meshes;
        std::unordered_map<uint32_t, LightImpl> lights;
        std::unordered_map<uint32_t, CameraImpl> cameras;

        void createBackGround(const char* fragmentShaderModule, std::vector<DescriptorInfo> const& descriptors) override;
        void deleteBackGround() override;
        Light* createLightSource() override;
        Mesh* createEmptyMesh() override;
        Camera* createCamera() override;

        void deleteMesh(Mesh* mesh) override;
        void deleteLightSource(Light* light) override;
        void deleteCamera(Camera* camera) override;

        void draw(VkCommandBuffer commandBuffer, CameraImpl* camera, RenderPass* pass, int currentFrame);

        void drawBackground(VkCommandBuffer commandBuffer, CameraImpl* camera, RenderPass* pass, int currentFrame);
        ~SceneImpl() override = default;

        void updateLight(uint32_t light);

        bool hasDynamicLights() const { return !lights.empty();};

    protected:
        void createImpl() override;
        void destroyImpl() override;




    };
}
#endif //TEST_EXE_VULGINESCENE_H
