//
// Created by Бушев Дмитрий on 16.07.2021.
//

#include "VulgineUI.h"
#include "Vulgine.h"

namespace Vulgine {

    ImGuiTextBuffer UserInterface::logBuf;
    void UserInterface::draw() {
        auto &vlg = *vlg_instance;

        //ImGui::ShowDemoWindow(nullptr);

        if (!opened)
            return;

        for(auto* window: windows)
            window->draw();

        if (aboutOpened)
            drawAboutWindow();

        if (systemPropertiesOpened)
            drawSystemPropertiesWindow();
        if (metricsViewerOpened)
            drawMetricsViewerWindow();

        if(logOpened)
            drawLogWindow();


        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("VulGine")) {
                if (ImGui::MenuItem("Object Inspector", "CTRL+Z", pObjectInspector->isOpened())) {
                    pObjectInspector->open();
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
                if (ImGui::MenuItem("Show log file", "F2")) {
                    logOpened = true;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Quit", "Alt + F4")) {
                    glfwSetWindowShouldClose(vlg.window.instance(), GLFW_TRUE);
                }
                ImGui::EndMenu();
            }



            if(ImGui::BeginMenu("Settings")){
                if (ImGui::MenuItem("MSAAx1", "", vlg.settings.msaa == VK_SAMPLE_COUNT_1_BIT)) {
                    vlg.updateMSAA(VK_SAMPLE_COUNT_1_BIT);
                }
                if (ImGui::MenuItem("MSAAx2", "", vlg.settings.msaa == VK_SAMPLE_COUNT_2_BIT)) {
                    vlg.updateMSAA(VK_SAMPLE_COUNT_2_BIT);
                }
                if (ImGui::MenuItem("MSAAx4", "", vlg.settings.msaa == VK_SAMPLE_COUNT_4_BIT)) {
                    vlg.updateMSAA(VK_SAMPLE_COUNT_4_BIT);
                }
                if (ImGui::MenuItem("MSAAx8", "", vlg.settings.msaa == VK_SAMPLE_COUNT_8_BIT)) {
                    vlg.updateMSAA(VK_SAMPLE_COUNT_8_BIT);
                }
                ImGui::Separator();
                if(ImGui::MenuItem("vsync", "", vlg.settings.vsync)){
                    vlg.toggleVsync();
                }
                if(ImGui::MenuItem("fullscreen", "", vlg.window.fullscreen)){
                    if(!vlg.window.fullscreen)
                        vlg.window.goFullscreen();
                    else
                        vlg.window.goWindowed();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

    }

    void UserInterface::drawObjectInspectorWindow() {
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
        if (ImGui::Begin("About", &aboutOpened)) {
            ImGui::Text(getStringVersion().c_str());
            ImGui::Separator();
            ImGui::Text("By Bushev Dmitry, 2021.");
        }

        ImGui::End();

    }

    void UserInterface::addLog(const char *log, ...) {
        va_list args;
        va_start(args, log);
        logBuf.appendfv(log, args);
        logBuf.append("\n");
        va_end(args);
    }

    void UserInterface::drawLogWindow() {
        auto &vlg = *vlg_instance;
        if (ImGui::Begin("Log", &logOpened)) {
            ImGui::BeginChild("output");
            ImGui::TextUnformatted(logBuf.begin(), logBuf.end());

            // autoscroll
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
        }

        ImGui::End();

    }

    UserInterface::UserInterface() {
        pObjectInspector = new ObjectInspector();
        windows.emplace_back(pObjectInspector);
    }

    UserInterface::~UserInterface() {
        for(auto* window: windows)
            delete window;
    }

    void ObjectInspector::draw() {

        if(!opened)
            return;

        auto &vlg = *vlg_instance;

        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Object Inspector", &opened)) {
            ImGui::End();
            return;
        }


        auto* selectedObjectTemp = selectedObject;

        bool selectedPresent = false;
        ImGui::BeginChild("left pane", ImVec2(150, 0), true);

        ObjectImpl::for_each([&selectedObjectTemp, &selectedPresent](ObjectImpl* object){
            bool selected = selectedObjectTemp == object;
            if(ImGui::Selectable(object->objectLabel().c_str(), selected)){
                selected = true;
                selectedObjectTemp = object;
            }
            if(selected)
                selectedPresent = true;
        });

        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("right pane", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

        if (selectedPresent) {
            selectedObject = selectedObjectTemp;
        } else {
            selectedObject = nullptr;
            ImGui::Text("Select object to display");
            ImGui::EndChild();
            ImGui::End();
            return;
        }


        ImGui::Text(selectedObject->objectLabel().c_str());
        ImGui::SameLine();
        ImGui::TextColored({128, 0, 0, 255}, selectedObject->typeName().c_str());
        ImGui::Separator();


        switch (selectedObject->type()) {
            case Object::Type::MATERIAL: {
                displayMaterialInfo();
                break;
            }
            case Object::Type::SCENE: {
                displaySceneInfo();
                break;
            }
            case Object::Type::IMAGE: {
                displayImageInfo();
                break;
            }
            case Object::Type::UBO: {
                displayUBOInfo();
                break;
            }
            default:
                ImGui::Text("Cannot display any information for this type of object yet");
                break;
        }
        ImGui::EndChild();
        ImGui::End();
    }

    void ObjectInspector::select(ObjectImpl* object){
        selectedObject = object;
    }


    void ObjectInspector::selectable(ObjectImpl *object) {
        std::string label = object ? object->objectLabel() : "null";
        if (ImGui::Selectable(label.c_str()) && object) {
            select(object);
        }
    }


    void ObjectInspector::displaySceneInfo() {
        static MeshImpl *selectedMesh = nullptr;
        bool selectedMeshPresent = false;
        auto &scene = *(dynamic_cast<SceneImpl*>(selectedObject));
        ImGui::BeginChild("Meshes", ImVec2(150, 200), true);

        for (auto &mesh: scene.meshes) {
            std::string label = mesh.second.objectLabel();


            if (ImGui::Selectable(label.c_str(), selectedMesh == &mesh.second)) {
                selectedMesh = &mesh.second;
            }

            if (selectedMesh == &mesh.second)
                selectedMeshPresent = true;
        }
        if (!selectedMeshPresent)
            selectedMesh = nullptr;

        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::Text("Total meshes: %d\nTotal cameras: %d\nTotal lights: %d", scene.meshes.size(), scene.cameras.size(), scene.lights.size());
        ImGui::Separator();

        if (selectedMesh) {
            auto &mesh = *selectedMesh;
            ImGui::Text(mesh.objectLabel().c_str());
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
                            selectable(dynamic_cast<ImageImpl*>(descriptor.image));
                            break;
                        }
                        case DescriptorInfo::Type::UNIFORM_BUFFER: {
                            ImGui::BulletText("Resource: ");
                            ImGui::SameLine();
                            selectable(dynamic_cast<UniformBufferImpl*>(descriptor.ubo));
                            break;
                        }
                        default:;
                    }
                    ImGui::Separator();
                }
            }

        }
    }

    void ObjectInspector::displayMaterialInfo() {
        auto &material = *dynamic_cast<MaterialImpl*>(selectedObject);
        if (ImGui::CollapsingHeader("Texture")) {
            ImGui::BulletText("Color Map: ");
            ImGui::SameLine();
            std::string label;

            auto *colorMap = dynamic_cast<ImageImpl*>(material.texture.colorMap);

            selectable(colorMap);

            ImGui::BulletText("Normal Map: ");

            ImGui::SameLine();

            auto *normalMap = dynamic_cast<ImageImpl*>(material.texture.normalMap);

            selectable(normalMap);

            ImGui::Separator();
        }


    }

    void ObjectInspector::displayImageInfo() {
        auto &image = *dynamic_cast<ImageImpl*>(selectedObject);
        std::string type;
        std::string dims;

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

        ImGui::Image((void*)(dynamic_cast<Image*>(selectedObject)), ImVec2(150, 150));
    }

    void ObjectInspector::displayUBOInfo() {
        auto &ubo = *dynamic_cast<UniformBufferImpl*>(selectedObject);;

        std::string type = ubo.dynamic ? "dynamic" : "static";

        ImGui::Text("type: %s", type.c_str());
        ImGui::Text("Size: %d", ubo.size);

    }

}