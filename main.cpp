//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/IVulgine.h"
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <imgui/imgui.h>

struct VertexAttribute{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct InstanceAttribute{
    glm::mat4 transform;
};

constexpr const int metaCubesize = 10;

InstanceAttribute instancesAttributes[metaCubesize * metaCubesize * metaCubesize] = {
        {glm::mat4(1.0f)}
};

InstanceAttribute standaloneCube = {glm::mat4(1.0f)};

VertexAttribute vertexAttributes[24] = {

        // upper face
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},

        // lower face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},

        // east face
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        // west face
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        // north face
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},

        // south face
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
                                       };

struct Camera{
    Vulgine::Camera* cameraImpl;

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool space = false;
        bool shift = false;
    } keys;


    glm::vec3 velocity = glm::vec3{0.0f};
    double inertia = 0.1f;
    double actingForce = 20.0f;
    double speed = 5.0f;

    void rotate(double dx, double dy, double dz){
        cameraImpl->rotation.x += dx;
        cameraImpl->rotation.y += dy;
        cameraImpl->rotation.z += dz;

        if(cameraImpl->rotation.x > 89.0f)
            cameraImpl->rotation.x = 89.0f;
        if(cameraImpl->rotation.x < -89.0f)
            cameraImpl->rotation.x = -89.0f;
        cameraImpl->update();
    }
    void update(double deltaT){

        float currentSpeed = glm::length(velocity);

        if(currentSpeed < 0.1f)
            velocity *= 0.0f;
        else
            velocity *= 1.0f - (1.5f) * deltaT / inertia;

        glm::vec3 camFront;
        camFront.x = cos(glm::radians(cameraImpl->rotation.x)) * sin(glm::radians(cameraImpl->rotation.y));
        camFront.y = -sin(glm::radians(cameraImpl->rotation.x));
        camFront.z = cos(glm::radians(cameraImpl->rotation.x)) * cos(glm::radians(cameraImpl->rotation.y));
        camFront = glm::normalize(-camFront);

        float acceleration = actingForce * deltaT / inertia;



        if (keys.up)
            velocity += camFront * acceleration;
        if (keys.down)
            velocity -= camFront * acceleration;
        if (keys.left)
            velocity -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * acceleration;
        if (keys.right)
            velocity += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * acceleration;
        if (keys.space)
            velocity += glm::vec3(0.0f, 1.0f, 0.0f) * acceleration;
        if (keys.shift)
            velocity -= glm::vec3(0.0f, 1.0f, 0.0f) * acceleration;

        cameraImpl->position += velocity * (float)deltaT;

        cameraImpl->update();
    };
};

void createSampleMesh(Vulgine::Mesh* mesh, Vulgine::Geometry* geometry, Vulgine::Material* material, Vulgine::UniformBuffer* ubo){
    Vulgine::VertexFormat format;
    format.perVertexAttributes = {Vulgine::AttributeFormat::RGB32SF, Vulgine::AttributeFormat::RGB32SF, Vulgine::AttributeFormat::RG32SF};
    format.perInstanceAttributes = {Vulgine::AttributeFormat::MAT4F};

    geometry->vertexFormat = format;
    geometry->vertexShader = "vert_color";
    mesh->vertices.pData = vertexAttributes;
    mesh->vertices.count = sizeof(vertexAttributes) / sizeof(VertexAttribute);
    mesh->instances.pData = instancesAttributes;
    mesh->instances.count = sizeof(instancesAttributes) / sizeof(InstanceAttribute);
    mesh->indices = {0, 1, 2, 0, 2, 3, 6, 5, 4, 7, 6, 4,
                     8, 9, 10, 8, 10, 11, 12, 14, 13, 12, 15, 14,
                     16, 17, 18, 16, 18, 19, 20, 23, 22, 20, 22, 21};

    Vulgine::Mesh::Primitive primitive{};

    primitive.material = material;

    primitive.startIdx = 0;

    primitive.indexCount = mesh->indices.size();

    mesh->primitives.push_back(primitive);

    Vulgine::DescriptorInfo info{};
    Vulgine::Descriptor desc{};

    info.binding = 0;
    desc.image = nullptr;
    desc.ubo = ubo;
    info.type = Vulgine::DescriptorInfo::Type::UNIFORM_BUFFER;

    mesh->descriptors.push_back(desc);
    geometry->descriptors.push_back(info);

    mesh->vertices.dynamic = false;
    mesh->instances.dynamic = false;

    mesh->geometry = geometry;

    geometry->create();

    mesh->create();
}

void createOffscreenProjectedMesh(Vulgine::Mesh* mesh, Vulgine::Geometry* geometry, Vulgine::Material* material){
    Vulgine::VertexFormat format;
    format.perVertexAttributes = {Vulgine::AttributeFormat::RGB32SF, Vulgine::AttributeFormat::RGB32SF, Vulgine::AttributeFormat::RG32SF};
    format.perInstanceAttributes = {Vulgine::AttributeFormat::MAT4F};

    geometry->vertexFormat = format;
    geometry->vertexShader = "vert_default";
    mesh->vertices.pData = vertexAttributes;
    mesh->vertices.count = sizeof(vertexAttributes) / sizeof(VertexAttribute);
    mesh->instances.pData = &standaloneCube;
    mesh->instances.count = 1;
    mesh->indices = {0, 1, 2, 0, 2, 3, 6, 5, 4, 7, 6, 4,
                     8, 9, 10, 8, 10, 11, 12, 14, 13, 12, 15, 14,
                     16, 17, 18, 16, 18, 19, 20, 23, 22, 20, 22, 21};

    Vulgine::Mesh::Primitive primitive{};

    primitive.material = material;

    primitive.startIdx = 0;

    primitive.indexCount = mesh->indices.size();

    mesh->primitives.push_back(primitive);

    mesh->vertices.dynamic = false;
    mesh->instances.dynamic = false;
    mesh->geometry = geometry;

    geometry->create();

    mesh->create();

}
int main(int argc, char** argv){

    Vulgine::initializeInfo.windowName = "HELLO THERE";
    Vulgine::initializeInfo.windowSize = {1200, 800};
    Vulgine::initializeInfo.enableVulkanValidationLayers = true;
    Vulgine::initializeInfo.vsync = false;
    Vulgine::initializeInfo.fullscreen = false;

    glm::vec4 shift;

    if(!Vulgine::Init()){
        std::cout << "Failed to init VulGine. Aborting execution." << std::endl;
        return -1;
    }

    auto* vulgine = Vulgine::Get();

    auto* scene = vulgine->initNewScene();

    scene->create();

    auto* sampler = vulgine->initNewSampler();

    sampler->create();

    auto* light1 = scene->createLightSource();

    light1->direction = glm::vec2{0.0f, 0.0f};
    light1->color = glm::vec3{1.0f, 0.5f, 0.2f};
    light1->update();

    auto* light2 = scene->createLightSource();

    light2->direction = glm::vec2{0.0f, 0.0f};
    light2->color = glm::vec3{1.0f, 1.0f, 1.0f};
    light2->update();

    auto* material = vulgine->initNewMaterial();



    material->setName("Wario Surface");

    auto* texture = vulgine->initNewImage();


    auto* ubo = vulgine->initNewUniformBuffer();

    ubo->dynamic = true;
    ubo->pData = &shift;
    ubo->size = sizeof(shift);
    ubo->create();

    if(!texture->loadFromFile("image.jpg", Vulgine::Image::FILE_FORMAT_JPEG)){
        Vulgine::Terminate();
        return 0;
    }


    material->texture.colorMap = texture;
    material->texture.sampler = sampler;

    material->create();

    Camera camera;

    camera.cameraImpl = scene->createCamera();

    for(int i = 0; i < metaCubesize; i++)
        for(int j = 0; j < metaCubesize; j++)
            for(int k = 0; k < metaCubesize; k++)
                instancesAttributes[i * metaCubesize * metaCubesize + j * metaCubesize + k].transform = glm::translate(glm::vec3{i * 2.0f, j * 2.0f, k * 2.0f});

    double timer = 0.0f, deltaT = 0.0f;

    vulgine->keyboardState.onKeyDown = [&camera, vulgine](int key){
        switch(key){

            case GLFW_KEY_W: camera.keys.up = true; break;
            case GLFW_KEY_S: camera.keys.down = true; break;
            case GLFW_KEY_A: camera.keys.left = true; break;
            case GLFW_KEY_D: camera.keys.right = true; break;
            case GLFW_KEY_SPACE: camera.keys.space = true; break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT: camera.keys.shift = true; break;
            case GLFW_KEY_C: {
                if(vulgine->mouseState.cursor.enabled)
                    vulgine->mouseState.disableCursor();
                else
                    vulgine->mouseState.enableCursor();

                break;
            }
            default: break;
        }

    };

    vulgine->keyboardState.onKeyUp = [&camera](int key){
        switch(key){
            case GLFW_KEY_W: camera.keys.up = false; break;
            case GLFW_KEY_S: camera.keys.down = false; break;
            case GLFW_KEY_A: camera.keys.left = false; break;
            case GLFW_KEY_D: camera.keys.right = false; break;
            case GLFW_KEY_SPACE: camera.keys.space = false; break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT: camera.keys.shift = false; break;

            default: break;
        }

    };

    vulgine->imgui.customGUI = [&camera](){
        ImGui::Begin("Camera State", nullptr);
        ImGui::Text("Velocity: %f", glm::length(camera.velocity));
        auto const& pos = camera.cameraImpl->position;
        ImGui::Text("Position: x:%.2f y:%.2f z:%.2f",pos.x, pos.y, pos.z);
        ImGui::End();
    };

    const double rotSpeed = 0.1f;

    vulgine->mouseState.onMouseMove = [&camera, vulgine, rotSpeed](double dx, double dy, double x, double y){
        if(!vulgine->mouseState.cursor.enabled)
            camera.rotate(-dy * rotSpeed, -dx * rotSpeed, 0);
    };


    auto* mesh = scene->createEmptyMesh();
    auto* geometry = vulgine->initNewGeometry();

    createSampleMesh(mesh, geometry, material, ubo);

    auto* renderPass = vulgine->initNewRenderPass();

    auto* offscreenRenderPass = vulgine->initNewRenderPass();

    renderPass->onscreen = true;
    renderPass->dependencies.push_back(offscreenRenderPass);

    offscreenRenderPass->getFrameBuffer()->width = 500;
    offscreenRenderPass->getFrameBuffer()->height = 500;

    auto* attachmentImage = offscreenRenderPass->getFrameBuffer()->addAttachment();
    //offscreenRenderPass->getFrameBuffer()->addAttachment(Vulgine::FrameBuffer::Type::DEPTH_STENCIL);

#if 1
    scene->createBackGround("frag_background_textured",
                            {{{Vulgine::DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER, 0},
                              {nullptr, sampler,
                              attachmentImage}}});
#endif
    auto* cube = scene->createEmptyMesh();
    cube->setName("Cube");

    auto* offscreenProjector = vulgine->initNewMaterial();
    offscreenProjector->texture.sampler = sampler;
    offscreenProjector->texture.colorMap = attachmentImage;
    offscreenProjector->create();
    offscreenProjector->setName("Offscreen Projector");

    standaloneCube.transform = glm::translate(glm::vec3{0.0f, -5.0f, 0.0f});

    auto* geometry1 = Vulgine::Get()->initNewGeometry();

    createOffscreenProjectedMesh(cube, geometry1, offscreenProjector);



    offscreenRenderPass->onscreen = false;
    offscreenRenderPass->camera = camera.cameraImpl;
    offscreenRenderPass->scene = scene;

    renderPass->scene = scene;
    renderPass->camera = camera.cameraImpl;

    vulgine->buildRenderPasses();

    bool skip = true;

    vulgine->onCycle = [&skip, &timer, &deltaT, vulgine, mesh, &camera, &shift, light1, light2, ubo](){
        if(skip)
            skip = false;
        else
            deltaT = vulgine->lastFrameTime();
        timer += deltaT;
        shift = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f) * (float)sin(timer);
        ubo->update();
        light1->update();
        light2->update();
#if 0
        for(int i = 0; i < metaCubesize; i++){
            glm::mat4 rotate = glm::rotate(glm::mat4(1.0f), (float)deltaT * (((i * 3 + 11) % 5) + 1),
                                           glm::normalize(glm::vec3{1.0f, (i * 7 + 19) % 13, (i * 23 + 5) % 37}));
            for(int j = 0; j < metaCubesize * metaCubesize; j++)
                instancesAttributes[j * metaCubesize + i ].transform = instancesAttributes[j * metaCubesize + i].transform * rotate;

        }

        mesh->updateInstanceBuffer();
#endif

        camera.update(deltaT);

    };

    while(vulgine->cycle());

    Vulgine::Terminate();

    return 0;
}
