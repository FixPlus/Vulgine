//
// Created by Бушев Дмитрий on 16.07.2021.
//

#include "VulgineUI.h"
#include "Vulgine.h"
namespace {
    ImVec2 mainMenuBarSize;
}
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
            mainMenuBarSize = ImGui::GetWindowSize();
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


    void UserInterface::drawMetricsViewerWindow() {
        auto &vlg = *vlg_instance;
        if (ImGui::Begin("Metrics Viewer", &metricsViewerOpened)) {
            ImGui::BulletText("FPS: %.1f (%.2fms)", vlg.fpsCounter.fps, 1000.0f / (float) vlg.fpsCounter.fps);
            ImGui::Separator();
            ImGui::BulletText("Meshes: %d", ObjectImpl::count(Object::Type::MESH));
            ImGui::BulletText("Images: %d", ObjectImpl::count(Object::Type::IMAGE));
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
        auto & io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.y / 3.0f, 0));
        ImGui::SetNextWindowPos(ImVec2(0, mainMenuBarSize.y));

        if (!ImGui::Begin("Object Inspector", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::End();
            return;
        }


        auto* selectedObjectTemp = selectedObject;

        bool selectedPresent = false;
        ImGui::BeginChild("Object list", ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, io.DisplaySize.y / 3.0f), true);

        ObjectImpl::for_each([&selectedObjectTemp, &selectedPresent](ObjectImpl* object){
            bool selected = selectedObjectTemp == object;
            if(ImGui::Selectable(object->objectLabel().c_str(), selected)){
                selected = true;
                selectedObjectTemp = object;
            }
            if(selected)
                selectedPresent = true;
        });

        if (selectedPresent) {
            selectedObject = selectedObjectTemp;
        } else {
            selectedObject = nullptr;
        }

        ImGui::EndChild();

        if(!ImGui::CollapsingHeader("Properties")) {
            ImGui::End();
            return;
        }

        ImGui::BeginChild("Properties viewer", ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, io.DisplaySize.y - ImGui::GetCursorPos().y - mainMenuBarSize.y), true);

        if(selectedObject == nullptr){
            ImGui::Text("No object selected");
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
            case Object::Type::MESH: {
                displayMeshInfo();
                break;
            }
            case Object::Type::RENDER_PASS:{
                displayRenderPassInfo();
                break;
            }
            case Object::Type::FRAME_BUFFER:{
                displayFrameBufferInfo();
                break;
            }
            case Object::Type::CAMERA: {
                displayCameraInfo();
                break;
            }
            case Object::Type::LIGHT:{
                displayLightInfo();
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
        auto &scene = *(dynamic_cast<SceneImpl*>(selectedObject));
        ImGui::BeginChild("Meshes", ImVec2(150, 200), true);

        for (auto &mesh: scene.meshes) {
            selectable(&mesh.second);
        }


        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::Text("Total meshes: %d\nTotal cameras: %d\nTotal lights: %d", scene.meshes.size(), scene.cameras.size(), scene.lights.size());

    }

    void ObjectInspector::displayMaterialInfo() {
        auto &material = *dynamic_cast<MaterialImpl*>(selectedObject);
        if (ImGui::CollapsingHeader("Texture")) {
            ImGui::BulletText("Color Map: ");
            ImGui::SameLine();
            std::string label;

            if(auto *colorMap = dynamic_cast<StaticImageImpl*>(material.texture.colorMap)) {
                selectable(colorMap);
            } else {
                selectable(dynamic_cast<DynamicImageImpl*>(material.texture.colorMap));
            }
            ImGui::BulletText("Normal Map: ");

            ImGui::SameLine();

            if(auto *normalMap = dynamic_cast<StaticImageImpl*>(material.texture.normalMap)) {
                selectable(normalMap);
            } else {
                selectable(dynamic_cast<DynamicImageImpl*>(material.texture.normalMap));
            }
            ImGui::Separator();
        }


    }

    void ObjectInspector::displayImageInfo() {
        VkImageCreateInfo imageInfo;

        if(auto* image = dynamic_cast<StaticImageImpl*>(selectedObject)){
            ImGui::Text("Static texture");

            imageInfo = image->image.imageInfo;
        }
        if(auto* image = dynamic_cast<DynamicImageImpl*>(selectedObject)){
            ImGui::Text("Dynamic texture");
            ImGui::Text("Image Count: %d", image->images.size());
            imageInfo = image->images.at(0).imageInfo;
        }
        std::string type;
        std::string dims;

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

    void ObjectInspector::displayMeshInfo() {
        auto &mesh = *dynamic_cast<MeshImpl*>(selectedObject);
        ImGui::Text("Parent: ");
        ImGui::SameLine();
        selectable(dynamic_cast<ObjectImpl*>(mesh.parent()));
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
                        if(auto* image = dynamic_cast<StaticImageImpl*>(descriptor.image))
                            selectable(image);
                        else {
                            selectable(dynamic_cast<DynamicImageImpl*>(descriptor.image));
                        }
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

    void ObjectInspector::displayRenderPassInfo() {
        auto* pRenderPass = dynamic_cast<RenderPassImpl*>(selectedObject);
        assert(pRenderPass && "Expected dynamic type match RenderPassImpl");
        auto& renderPass = *pRenderPass;

        ImGui::Text("Draws: ");
        ImGui::SameLine();
        selectable(dynamic_cast<SceneImpl*>(renderPass.scene));

        ImGui::Text("From pov of: ");
        ImGui::SameLine();
        selectable(dynamic_cast<CameraImpl*>(renderPass.camera));

        ImGui::Text("To: ");
        ImGui::SameLine();
        selectable(&renderPass.frameBuffer);

        ImGui::Separator();

        ImGui::Text("Dependencies:");
        for(auto* depend: renderPass.dependencies)
            selectable(dynamic_cast<RenderPassImpl*>(depend));

    }

    void ObjectInspector::displayFrameBufferInfo() {
        auto* pFrameBuffer = dynamic_cast<FrameBufferImpl*>(selectedObject);
        assert(pFrameBuffer && "Expected dynamic type match FrameBufferImpl");
        auto& frameBuffer = *pFrameBuffer;

        if(frameBuffer.renderPass->onscreen){
            ImGui::Text("Type: onscreen");

        } else{
            ImGui::Text("Type: offscreen");
            ImGui::Separator();
            ImGui::Text("Attachments: ");
            for(auto& imageMapped: frameBuffer.attachmentsImages){
                auto* image = &imageMapped.second;
                ImGui::Text("Attachment #%d: ", imageMapped.first);
                ImGui::SameLine();
                selectable(image);
            }
        }

    }

    void ObjectInspector::displayCameraInfo() {
        auto* pCamera = dynamic_cast<CameraImpl*>(selectedObject);
        assert(pCamera && "Expected dynamic type match CameraImpl");
        auto& camera = *pCamera;

        ImGui::DragFloat3("Position", reinterpret_cast<float*>(&camera.position));
        ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&camera.rotation));
    }

    void ObjectInspector::displayLightInfo() {
        auto* pLight = dynamic_cast<LightImpl*>(selectedObject);
        assert(pLight && "Expected dynamic type match LightImpl");
        auto& light = *pLight;


        ImGui::DragFloat2("Direction", reinterpret_cast<float*>(&light.direction));
        ImGui::ColorPicker3("Color", reinterpret_cast<float*>(&light.color));
    }

}