//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/IVulgine.h"
#include <glm/vec4.hpp>
#include <cmath>

struct VertexAttribute{
    glm::vec4 pos;
    glm::vec4 color;
};

VertexAttribute vertexAttributes[4] = {{{-0.5f, -0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 0.0f}},
                                       {{-0.5f, 0.5f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 0.0f}},
                                       {{0.5f, 0.5f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 0.0f}},
                                       {{0.5f, -0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f, 0.0f}}};

void createSampleMesh(Vulgine::Mesh* mesh, Vulgine::Material* material){
    Vulgine::VertexFormat format;
    format.perVertexAttributes = {Vulgine::AttributeFormat::RGBA32SF, Vulgine::AttributeFormat::RGBA32SF};

    mesh->vertexFormat = format;
    mesh->vertices.pData = vertexAttributes;
    mesh->vertices.count = 4;
    mesh->indices = {0, 1, 2, 0, 2, 3};

    material->vertexFormat = format;

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
    Vulgine::initializeInfo.enableVulkanValidationLayers = true;

    auto* vulgine = Vulgine::Vulgine::createInstance();

    auto* scene = vulgine->initNewScene();

    auto* material = vulgine->initNewMaterial();

    material->vertexShader = "vert_default";

    auto* camera = scene->createCamera();

    auto* mesh = scene->createEmptyMesh();

    createSampleMesh(mesh, material);

    Vulgine::RenderTarget renderTarget = {Vulgine::RenderTarget::COLOR, Vulgine::RenderTarget::SCREEN};

    vulgine->updateRenderTaskQueue({{scene, camera, {renderTarget}}});

    double timer = 0.0f;

    while(vulgine->cycle()){
        timer += vulgine->lastFrameTime();
        vertexAttributes[0].pos = {0.1 * sin(timer) - 0.5f, 0.1 * cos(timer) - 0.5f, 0.0f, 0.0f};
        mesh->updateVertexBuffer();
    };



    //vulgine->deleteScene(scene);

    Vulgine::Vulgine::freeInstance(vulgine);

    return 0;
}
