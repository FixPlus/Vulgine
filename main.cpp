//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/IVulgine.h"
#include <glm/vec4.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>

struct VertexAttribute{
    glm::vec4 pos;
    glm::vec4 color;
};

VertexAttribute vertexAttributes[4] = {{{-0.5f, -0.5f, -10.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
                                       {{-0.5f, 0.5f, -10.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
                                       {{0.5f, 0.5f, -10.0f, 1.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},
                                       {{0.5f, -0.5f, -10.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 0.0f}}};

struct Camera{
    Vulgine::Camera* cameraImpl;

    struct
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
    } keys;

    double velocity = 5.0f;

    void rotate(double dx, double dy, double dz){
        cameraImpl->rotation.x += dx;
        cameraImpl->rotation.y += dy;
        cameraImpl->rotation.z += dz;

        cameraImpl->update();
    }
    void update(double deltaT){

        glm::vec3 camFront;
        camFront.x = -cos(glm::radians(cameraImpl->rotation.x)) * sin(glm::radians(cameraImpl->rotation.y));
        camFront.y = sin(glm::radians(cameraImpl->rotation.x));
        camFront.z = cos(glm::radians(cameraImpl->rotation.x)) * cos(glm::radians(cameraImpl->rotation.y));
        camFront = glm::normalize(camFront);

        float moveSpeed = deltaT * velocity;

        if (keys.up)
            cameraImpl->position += camFront * moveSpeed;
        if (keys.down)
            cameraImpl->position -= camFront * moveSpeed;
        if (keys.left)
            cameraImpl->position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
        if (keys.right)
            cameraImpl->position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

        cameraImpl->update();
    };
};

void createSampleMesh(Vulgine::Mesh* mesh, Vulgine::Material* material){
    Vulgine::VertexFormat format;
    format.perVertexAttributes = {Vulgine::AttributeFormat::RGBA32SF, Vulgine::AttributeFormat::RGBA32SF};

    mesh->vertexFormat = format;
    mesh->vertices.pData = vertexAttributes;
    mesh->vertices.count = 4;
    mesh->indices = {0, 1, 2, 0, 2, 3};

    Vulgine::Mesh::Primitive primitive{};

    primitive.material = material;

    primitive.startIdx = 0;

    primitive.indexCount = mesh->indices.size();

    mesh->primitives.push_back(primitive);

    mesh->vertices.dynamic = true;

    mesh->create();
}

int main(int argc, char** argv){

    Vulgine::initializeInfo.windowName = "HELLO THERE";
    Vulgine::initializeInfo.windowSize = {1200, 800};
    Vulgine::initializeInfo.enableVulkanValidationLayers = false;
    Vulgine::initializeInfo.vsync = false;
    Vulgine::initializeInfo.fullscreen = false;

    auto* vulgine = Vulgine::Vulgine::createInstance();

    auto* scene = vulgine->initNewScene();

    auto* material = vulgine->initNewMaterial();

    material->vertexShader = "vert_default";

    Camera camera;

    camera.cameraImpl = scene->createCamera();



    double timer = 0.0f, deltaT = 0.0f;

    vulgine->keyboardState.onKeyDown = [&camera, vulgine](int key){
        switch(key){

            case GLFW_KEY_W: camera.keys.up = true; break;
            case GLFW_KEY_S: camera.keys.down = true; break;
            case GLFW_KEY_A: camera.keys.left = true; break;
            case GLFW_KEY_D: camera.keys.right = true; break;
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

            default: break;
        }

    };

    const double rotSpeed = 0.1f;

    vulgine->mouseState.onMouseMove = [&camera, vulgine, rotSpeed](double dx, double dy, double x, double y){
        if(!vulgine->mouseState.cursor.enabled)
            camera.rotate(-dy * rotSpeed, dx * rotSpeed, 0);
    };


    auto* mesh = scene->createEmptyMesh();

    createSampleMesh(mesh, material);

    Vulgine::RenderTarget renderTarget = {Vulgine::RenderTarget::COLOR, Vulgine::RenderTarget::SCREEN};

    vulgine->updateRenderTaskQueue({{scene, camera.cameraImpl, {renderTarget}}});



    while(vulgine->cycle()){
        deltaT = vulgine->lastFrameTime();
        timer += deltaT;
        vertexAttributes[0].pos = {0.1 * sin(timer) - 0.5f, 0.1 * cos(timer) - 0.5f, -10.0f, 1.0f};
        vertexAttributes[1].color = {0.5 * sin(timer) + 0.5f, 0.5 * cos(timer) + 0.5f, 0.0f, 1.0f};
        mesh->updateVertexBuffer();
        camera.update(deltaT);
        //std::cout << "rot.x: " << camera->rotation.x << "; rot.y: " << camera->rotation.y << std::endl;
    };

    Vulgine::Vulgine::freeInstance(vulgine);

    return 0;
}
