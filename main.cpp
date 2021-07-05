//
// Created by Бушев Дмитрий on 01.07.2021.
//

#include "include/IVulgine.h"


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

    Vulgine::Mesh::Primitive primitive;

    primitive.material = material;

    mesh->primitives.push_back(primitive);

    Vulgine::RenderTarget renderTarget = {Vulgine::RenderTarget::COLOR, Vulgine::RenderTarget::SCREEN};

    vulgine->updateRenderTaskQueue({{scene, camera, {renderTarget}}});

    while(vulgine->cycle());



    //vulgine->deleteScene(scene);

    Vulgine::Vulgine::freeInstance(vulgine);

    return 0;
}
