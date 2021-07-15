//
// Created by Бушев Дмитрий on 16.07.2021.
//

#include "VulgineUI.h"
#include "Vulgine.h"

namespace Vulgine {

    void UserInterface::draw() {
        auto &vlg = *vlg_instance;

        //ImGui::ShowDemoWindow(nullptr);

        if (!opened)
            return;

        if (objectInspectorOpened)
            drawObjectInspectorWindow();
        if (aboutOpened)
            drawAboutWindow();

        if (systemPropertiesOpened)
            drawSystemPropertiesWindow();
        if (metricsViewerOpened)
            drawMetricsViewerWindow();


        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("VulGine")) {
                if (ImGui::MenuItem("Object Inspector", "CTRL+Z", objectInspectorOpened)) {
                    objectInspectorOpened = true;
                }
                if (ImGui::MenuItem("Metrics Viewer", "CTRL+X", metricsViewerOpened)) {
                    metricsViewerOpened = true;
                }
                if (ImGui::MenuItem("System Properties", "CTRL+Y", systemPropertiesOpened)) {
                    systemPropertiesOpened = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Show about", "F1")) {
                    aboutOpened = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

    }

    void UserInterface::drawObjectInspectorWindow() {

        auto &vlg = *vlg_instance;

        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Object Inspector", &objectInspectorOpened)) {
            ImGui::End();
            return;
        }

        static int selectedType = -1;

        static union SelectedObject {
            MaterialImpl *material;
            SceneImpl *scene;
            ImageImpl *image;
            UniformBuffer *ubo;
        } selectedObject;

        int selectedTypeTemp = selectedType;
        SelectedObject selectedObjectTemp = selectedObject;

        bool selectedPresent = false;
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);

        vlg.materials.iterate([&selectedTypeTemp, &selectedObjectTemp, &selectedPresent](MaterialImpl &material) {
            std::string label = "Material #" + std::to_string(material.id());
            bool selected = selectedTypeTemp == 0 && selectedObjectTemp.material == &material;
            if (ImGui::Selectable(label.c_str(), selected)) {
                selectedTypeTemp = 0;
                selectedObjectTemp.material = &material;
                selected = true;
            }
            if (selected)
                selectedPresent = true;
        });

        vlg.scenes.iterate([&selectedTypeTemp, &selectedObjectTemp, &selectedPresent](SceneImpl &scene) {
            std::string label = "Scene #" + std::to_string(scene.id());
            bool selected = selectedTypeTemp == 1 && selectedObjectTemp.scene == &scene;
            if (ImGui::Selectable(label.c_str(), selected)) {
                selectedTypeTemp = 1;
                selectedObjectTemp.scene = &scene;
                selected = true;
            }
            if (selected)
                selectedPresent = true;
        });

        vlg.images.iterate([&selectedTypeTemp, &selectedObjectTemp, &selectedPresent](ImageImpl &image) {
            std::string label = "Image #" + std::to_string(image.id());
            bool selected = selectedTypeTemp == 2 && selectedObjectTemp.image == &image;
            if (ImGui::Selectable(label.c_str(), selected)) {
                selectedTypeTemp = 2;
                selectedObjectTemp.image = &image;
                selected = true;
            }
            if (selected)
                selectedPresent = true;
        });

        vlg.uniformBuffers.iterate([&selectedTypeTemp, &selectedObjectTemp, &selectedPresent](UniformBufferImpl &ubo) {
            std::string label = "Uniform Buffer #" + std::to_string(ubo.id());
            bool selected = selectedTypeTemp == 3 && selectedObjectTemp.ubo == &ubo;
            if (ImGui::Selectable(label.c_str(), selected)) {
                selectedTypeTemp = 3;
                selectedObjectTemp.ubo = &ubo;
                selected = true;
            }
            if (selected)
                selectedPresent = true;
        });

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("right pane", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

        if (selectedPresent) {
            selectedObject = selectedObjectTemp;
            selectedType = selectedTypeTemp;
        } else {
            selectedType = -1;
        }

        switch (selectedType) {
            case 0: {
                auto &material = *selectedObject.material;
                ImGui::Text("Material #%d", material.id());
                ImGui::Separator();
                if (ImGui::CollapsingHeader("Texture")) {
                    ImGui::BulletText("Color Map: ");
                    ImGui::SameLine();
                    std::string label;

                    auto *colorMap = material.texture.colorMap;

                    if (colorMap)
                        label = "Image #" + std::to_string(colorMap->id());
                    else
                        label = "null";

                    if (ImGui::Selectable(label.c_str()) && colorMap) {
                        selectedObject.image = dynamic_cast<ImageImpl *>(colorMap);
                        selectedType = 2;
                    }

                    ImGui::BulletText("Normal Map: ");

                    ImGui::SameLine();

                    auto *normalMap = material.texture.normalMap;

                    if (normalMap)
                        label = "Image #" + std::to_string(normalMap->id());
                    else
                        label = "null";

                    if (ImGui::Selectable(label.c_str()) && normalMap) {
                        selectedObject.image = dynamic_cast<ImageImpl *>(normalMap);
                        selectedType = 2;
                    }

                    ImGui::Separator();
                }

                break;
            }
            case 1: {
                static MeshImpl *selectedMesh = nullptr;
                bool selectedMeshPresent = false;
                auto &scene = *selectedObject.scene;
                ImGui::Text("Scene #%d", scene.id());
                ImGui::BeginChild("Meshes", ImVec2(150, 200), true);

                for (auto &mesh: scene.meshes) {
                    std::string label = "Mesh #" + std::to_string(mesh.first);


                    if (ImGui::Selectable(label.c_str(), selectedMesh == &mesh.second)) {
                        selectedMesh = &mesh.second;
                    }

                    if (selectedMesh == &mesh.second)
                        selectedMeshPresent = true;
                }
                if (!selectedMeshPresent)
                    selectedMesh = nullptr;

                ImGui::EndChild();

                if (selectedMesh) {
                    auto &mesh = *selectedMesh;
                    ImGui::Text("Mesh #%d", mesh.id());
                    ImGui::Separator();
                    if (ImGui::CollapsingHeader("Vertex Input Layout")) {
                        ImGui::Text("Per-Vertex:");
                        int location = 0;
                        auto iterate = [&location](AttributeFormat attr) {
                            std::string format;
                            int curLoc = location;
                            switch (attr) {
                                case AttributeFormat::RGBA32SF: {
                                    format = "vec4";
                                    location++;
                                    break;
                                }
                                case AttributeFormat::RGB32SF: {
                                    format = "vec3";
                                    location++;
                                    break;
                                }
                                case AttributeFormat::RG32SF: {
                                    format = "vec2";
                                    location++;
                                    break;
                                }
                                case AttributeFormat::R32SF: {
                                    format = "float";
                                    location++;
                                    break;
                                }
                                case AttributeFormat::MAT4F: {
                                    format = "mat4";
                                    location += 4;
                                    break;
                                }
                                default:
                                    format = "???";
                            }

                            ImGui::BulletText("location = %d, %s", curLoc, format.c_str());

                        };

                        for (auto attr: mesh.vertexStageInfo.vertexFormat.perVertexAttributes)
                            iterate(attr);

                        if (mesh.vertexStageInfo.vertexFormat.perVertexAttributes.empty())
                            ImGui::Text("empty");

                        ImGui::Separator();

                        ImGui::Text("Per-Instance:");

                        for (auto attr: mesh.vertexStageInfo.vertexFormat.perInstanceAttributes)
                            iterate(attr);

                        if (mesh.vertexStageInfo.vertexFormat.perInstanceAttributes.empty())
                            ImGui::Text("empty");

                    }
                    if (ImGui::CollapsingHeader("Vertices")) {
                        std::string type = mesh.vertices.dynamic ? "dynamic" : "static";

                        ImGui::Text("Buffer type: %s", type.c_str());
                        ImGui::Text("Count: %d", mesh.vertices.count);
                        ImGui::Text("Size: %d bytes",
                                    mesh.vertexStageInfo.vertexFormat.perVertexSize() * mesh.vertices.count);
                    }
                    if (ImGui::CollapsingHeader("Instances")) {
                        std::string type = mesh.instances.dynamic ? "dynamic" : "static";

                        ImGui::Text("Buffer type: %s", type.c_str());
                        ImGui::Text("Count: %d", mesh.instances.count);
                        ImGui::Text("Size: %d bytes",
                                    mesh.vertexStageInfo.vertexFormat.perInstanceSize() * mesh.instances.count);

                    }
                    if (ImGui::CollapsingHeader("Descriptor layout")) {
                        for (auto const &descriptor: mesh.vertexStageInfo.descriptors) {
                            std::string type;
                            switch (descriptor.type) {
                                case DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER: {
                                    type = "combined image sampler";
                                    break;
                                }
                                case DescriptorInfo::Type::UNIFORM_BUFFER: {
                                    type = "uniform buffer";
                                    break;
                                }
                                default:
                                    type = "unknown";
                            }
                            ImGui::Text("Binding %d: %s", descriptor.binding, type.c_str());
                            switch (descriptor.type) {
                                case DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER: {
                                    ImGui::BulletText("Resource: ");
                                    ImGui::SameLine();
                                    std::string label = "Image #" + std::to_string(descriptor.image->id());
                                    if (ImGui::Selectable(label.c_str())) {
                                        selectedObject.image = dynamic_cast<ImageImpl *>(descriptor.image);
                                        selectedType = 2;
                                    }
                                    break;
                                }
                                case DescriptorInfo::Type::UNIFORM_BUFFER: {
                                    ImGui::BulletText("Resource: ");
                                    ImGui::SameLine();
                                    std::string label = "Uniform Buffer #" + std::to_string(descriptor.ubo->id());
                                    if (ImGui::Selectable(label.c_str())) {
                                        selectedObject.ubo = dynamic_cast<UniformBufferImpl *>(descriptor.ubo);
                                        selectedType = 3;
                                    }
                                    break;
                                }
                                default:;
                            }
                            ImGui::Separator();
                        }
                    }

                }

                break;
            }
            case 2: {
                auto &image = *selectedObject.image;
                std::string type;
                std::string dims;
                ImGui::Text("Image #%d", image.id());
                ImGui::Separator();
                auto const &imageInfo = image.image.imageInfo;
                switch (imageInfo.imageType) {
                    case VK_IMAGE_TYPE_1D:
                        type = "1D";
                        dims = std::to_string(imageInfo.extent.width);
                        break;
                    case VK_IMAGE_TYPE_2D:
                        type = "2D";
                        dims = std::to_string(imageInfo.extent.width) + " x " +
                               std::to_string(imageInfo.extent.height);
                        break;
                    case VK_IMAGE_TYPE_3D:
                        type = "3D";
                        dims = std::to_string(imageInfo.extent.width) + " x " +
                               std::to_string(imageInfo.extent.height) + " x " +
                               std::to_string(imageInfo.extent.depth);
                        break;
                    default:
                        type = "unknown";
                        dims = "???";
                        break;
                }

                ImGui::Text("Type: %s", type.c_str());
                ImGui::Text("Dimensions: %s", dims.c_str());
                std::string format;

                switch (imageInfo.format) {
                    case VK_FORMAT_R32G32B32A32_SFLOAT:
                        format = "fRGBA32";
                        break;
                    case VK_FORMAT_R32G32B32_SFLOAT:
                        format = "fRGB32";
                        break;
                    case VK_FORMAT_R32G32_SFLOAT:
                        format = "fRG32";
                        break;
                    case VK_FORMAT_R32_SFLOAT:
                        format = "fR32";
                        break;
                    case VK_FORMAT_R32G32B32A32_SINT:
                        format = "iRGBA32";
                        break;
                    case VK_FORMAT_R32G32B32A32_UINT:
                        format = "uiRGBA32";
                        break;
                    case VK_FORMAT_R32G32B32_SINT:
                        format = "iRGB32";
                        break;
                    case VK_FORMAT_R32G32B32_UINT:
                        format = "uiRGB32";
                        break;
                    case VK_FORMAT_R8G8B8A8_SRGB:
                        format = "sRGBA";
                        break;
                    case VK_FORMAT_R8G8B8A8_SINT:
                        format = "iRGBA";
                        break;
                    case VK_FORMAT_R8G8B8A8_UINT:
                        format = "uiRGBA";
                        break;
                    case VK_FORMAT_R8G8B8A8_UNORM:
                        format = "unRGBA";
                        break;
                    case VK_FORMAT_D24_UNORM_S8_UINT:
                        format = "D24(unorm), S8(uint)";
                        break;
                    default:
                        format = "???";
                        break;
                }

                ImGui::Text("Format: %s", format.c_str());

                ImGui::Text("Mip levels: %d", imageInfo.mipLevels);
                ImGui::Text("Samples: %d", imageInfo.samples);
                break;
            }
            case 3: {
                auto &ubo = *selectedObject.ubo;

                ImGui::Text("Uniform Buffer #%d", ubo.id());
                ImGui::Separator();

                std::string type = ubo.dynamic ? "dynamic" : "static";

                ImGui::Text("type: %s", type.c_str());
                ImGui::Text("Size: %d", ubo.size);

                break;
            }
            default:
                break;
        }
        ImGui::EndChild();
        ImGui::End();
    }

    void UserInterface::drawMetricsViewerWindow() {
        auto &vlg = *vlg_instance;
        if (ImGui::Begin("Metrics Viewer", &metricsViewerOpened)) {
            ImGui::BulletText("FPS: %.1f (%.2fms)", vlg.fpsCounter.fps, 1000.0f / (float) vlg.fpsCounter.fps);
            ImGui::Separator();
            ImGui::BulletText("Meshes: %d", MeshImpl::count());
            ImGui::BulletText("Images: %d", vlg.images.size());
        }

        ImGui::End();
    }

    void UserInterface::drawSystemPropertiesWindow() {
        auto &vlg = *vlg_instance;
        if (ImGui::Begin("System properties", &systemPropertiesOpened)) {
            auto const &props = vlg.device->properties;
            ImGui::BulletText("Vulkan API version: %d.%d", props.apiVersion >> 22, (props.apiVersion >> 12) & 1023);
            ImGui::BulletText("Driver version: %d", props.driverVersion);
            ImGui::BulletText("GPU: %s", props.deviceName);
        }

        ImGui::End();

    }

    void UserInterface::drawAboutWindow() {
        auto &vlg = *vlg_instance;
        if (ImGui::Begin("System properties", &aboutOpened)) {
            ImGui::Text(getStringVersion().c_str());
            ImGui::Separator();
            ImGui::Text("By Bushev Dmitry, 2021.");
        }

        ImGui::End();

    }

}