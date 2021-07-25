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
        auto &vlg = GetImpl();

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
        auto &vlg = GetImpl();
        if (ImGui::Begin("Metrics Viewer", &metricsViewerOpened)) {
            ImGui::BulletText("FPS: %.1f (%.2fms)", vlg.fpsCounter.fps, 1000.0f / (float) vlg.fpsCounter.fps);
            ImGui::Separator();
            ImGui::BulletText("Meshes: %d", ObjectImpl::count(Object::Type::MESH));
            ImGui::BulletText("Images: %d", ObjectImpl::count(Object::Type::IMAGE));
        }

        ImGui::End();
    }

    void UserInterface::drawSystemPropertiesWindow() {
        auto &vlg = GetImpl();
        if (ImGui::Begin("System properties", &systemPropertiesOpened)) {
            auto const &props = vlg.device->properties;
            ImGui::BulletText("Vulkan API version: %d.%d", props.apiVersion >> 22, (props.apiVersion >> 12) & 1023);
            ImGui::BulletText("Driver version: %d", props.driverVersion);
            ImGui::BulletText("GPU: %s", props.deviceName);
        }

        ImGui::End();

    }

    void UserInterface::drawAboutWindow() {
        auto &vlg = GetImpl();
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
        auto &vlg = GetImpl();
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

        auto &vlg = GetImpl();
        auto & io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.y / 3.0f, 0));
        ImGui::SetNextWindowPos(ImVec2(0, mainMenuBarSize.y));

        if (!ImGui::Begin("Object Inspector", &opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
            ImGui::End();
            return;
        }


        auto selectedObjectTemp = selectedObject;

        bool selectedPresent = false;
        ImGui::BeginChild("Object list", ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, io.DisplaySize.y / 3.0f), true);

        ObjectImpl::for_each([&selectedObjectTemp, &selectedPresent](ObjectImpl* object){
            bool selected = selectedObjectTemp.has_value() && selectedObjectTemp == object->id();
            if(ImGui::Selectable(object->objectLabel().c_str(), selected)){
                selected = true;
                selectedObjectTemp = object->id();
            }
            if(selected)
                selectedPresent = true;
        });

        if (selectedPresent) {
            selectedObject = selectedObjectTemp;
        } else {
            selectedObject.reset();
        }

        ImGui::EndChild();

        if(!ImGui::CollapsingHeader("Properties")) {
            ImGui::End();
            return;
        }

        ImGui::BeginChild("Properties viewer", ImVec2(ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, io.DisplaySize.y - ImGui::GetCursorPos().y - mainMenuBarSize.y), true);

        if(!selectedObject.has_value()){
            ImGui::Text("No object selected");
            ImGui::EndChild();
            ImGui::End();
            return;
        }

        auto* selectedObj = ObjectImpl::get(selectedObject.value());

        ImGui::Text(selectedObj->objectLabel().c_str());
        ImGui::SameLine();
        ImGui::TextColored({128, 0, 0, 255}, selectedObj->typeName().c_str());
        ImGui::Separator();


        switch (selectedObj->type()) {
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
            case Object::Type::GEOMETRY: {
                displayGeometryInfo();
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

    void ObjectInspector::select(uint32_t id){
        selectedObject = id;
    }


    void ObjectInspector::selectable(ObjectImpl *object) {
        std::string label = object ? object->objectLabel() : "null";
        if (ImGui::Selectable(label.c_str()) && object) {
            select(object->id());
        }
    }


    void ObjectInspector::displaySceneInfo() {
        auto &scene = *(dynamic_cast<SceneImpl*>(ObjectImpl::get(selectedObject.value())));
        ImGui::BeginChild("Meshes", ImVec2(150, 200), true);

        for (auto &mesh: scene.drawList) {
            selectable(dynamic_cast< ObjectImpl *>(mesh.get()));
        }


        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::Text("Total meshes: %d\nTotal cameras: %d\nTotal lights: %d", scene.drawList.size(), scene.cameras.size(), scene.lights.size());

    }

    void ObjectInspector::displayMaterialInfo() {
        auto &material = *dynamic_cast<MaterialImpl*>(ObjectImpl::get(selectedObject.value()));

        if(ImGui::SliderFloat("Specular", &material.specular, 0.0f, 1.0f)){
            material.update();
        }
        if (ImGui::CollapsingHeader("Texture")) {
            ImGui::BulletText("Color Map: ");
            ImGui::SameLine();
            std::string label;

            if(auto *colorMap = dynamic_cast<StaticImageImpl*>(material.texture.colorMap.get())) {
                selectable(colorMap);
            } else {
                selectable(dynamic_cast<DynamicImageImpl*>(material.texture.colorMap.get()));
            }
            ImGui::BulletText("Normal Map: ");

            ImGui::SameLine();

            if(auto *normalMap = dynamic_cast<StaticImageImpl*>(material.texture.normalMap.get())) {
                selectable(normalMap);
            } else {
                selectable(dynamic_cast<DynamicImageImpl*>(material.texture.normalMap.get()));
            }
            ImGui::Separator();
        }


    }

    void ObjectInspector::displayImageInfo() {
        VkImageCreateInfo imageInfo;

        if(auto* image = dynamic_cast<StaticImageImpl*>(ObjectImpl::get(selectedObject.value()))){
            ImGui::Text("Static texture");

            imageInfo = image->image.imageInfo;
        }
        if(auto* image = dynamic_cast<DynamicImageImpl*>(ObjectImpl::get(selectedObject.value()))){
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

        ImGui::Image((void*)(dynamic_cast<Image*>(ObjectImpl::get(selectedObject.value()))), ImVec2(150, 150));
    }

    void ObjectInspector::displayUBOInfo() {
        auto &ubo = *dynamic_cast<UniformBufferImpl*>(ObjectImpl::get(selectedObject.value()));;

        std::string type = ubo.dynamic ? "dynamic" : "static";

        ImGui::Text("type: %s", type.c_str());
        ImGui::Text("Size: %d", ubo.size);

    }

    void ObjectInspector::displayMeshInfo() {
        auto &mesh = *dynamic_cast<MeshImpl*>(ObjectImpl::get(selectedObject.value()));
        ImGui::Text("Geometry: ");
        ImGui::SameLine();
        selectable(dynamic_cast<ObjectImpl*>(mesh.geometry.get()));

        if (ImGui::CollapsingHeader("Vertices")) {
            std::string type = mesh.vertices.dynamic ? "dynamic" : "static";

            if(mesh.geometry) {
                ImGui::Text("Buffer type: %s", type.c_str());
                ImGui::Text("Count: %d", mesh.vertices.count);
                ImGui::Text("Size: %d bytes",
                            mesh.geometry->vertexFormat.perVertexSize() * mesh.vertices.count);
            } else {
                ImGui::Text("Empty");
            }
        }
        if (ImGui::CollapsingHeader("Instances")) {
            std::string type = mesh.instances.dynamic ? "dynamic" : "static";
            if(mesh.geometry) {
                ImGui::Text("Buffer type: %s", type.c_str());
                ImGui::Text("Count: %d", mesh.instances.count);
                ImGui::Text("Size: %d bytes",
                            mesh.geometry->vertexFormat.perInstanceSize() * mesh.instances.count);
            } else {
                ImGui::Text("Empty");
            }

        }

        if(ImGui::CollapsingHeader("Geometry Descriptors")) {
            auto descIt = mesh.descriptors.begin();
            for (auto const &descriptor: mesh.geometry->descriptors) {
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
                ImGui::Separator();
                switch (descriptor.type) {
                    case DescriptorInfo::Type::COMBINED_IMAGE_SAMPLER: {
                        ImGui::BulletText("Resource: ");
                        ImGui::SameLine();
                        if (auto *image = dynamic_cast<StaticImageImpl *>(descIt->image.get()))
                            selectable(image);
                        else {
                            selectable(dynamic_cast<DynamicImageImpl *>(descIt->image.get()));
                        }
                        break;
                    }
                    case DescriptorInfo::Type::UNIFORM_BUFFER: {
                        ImGui::BulletText("Resource: ");
                        ImGui::SameLine();
                        selectable(dynamic_cast<UniformBufferImpl *>(descIt->ubo.get()));
                        break;
                    }
                    default:;
                }

                ++descIt;
            }

        }


    }

    void ObjectInspector::displayRenderPassInfo() {
        auto* pRenderPass = dynamic_cast<RenderPassImpl*>(ObjectImpl::get(selectedObject.value()));
        assert(pRenderPass && "Expected dynamic type match RenderPassImpl");
        auto& renderPass = *pRenderPass;

        ImGui::Text("Draws: ");
        ImGui::SameLine();
        selectable(dynamic_cast<SceneImpl*>(renderPass.scene.get()));

        ImGui::Text("From pov of: ");
        ImGui::SameLine();
        selectable(dynamic_cast<CameraImpl*>(renderPass.camera.get()));

        ImGui::Text("To: ");
        ImGui::SameLine();
        selectable(&renderPass.frameBuffer);

        ImGui::Separator();

        ImGui::Text("Dependencies:");
        for(auto const& depend: renderPass.dependencies)
            selectable(dynamic_cast<RenderPassImpl*>(depend.lock().get()));

    }

    void ObjectInspector::displayFrameBufferInfo() {
        auto* pFrameBuffer = dynamic_cast<FrameBufferImpl*>(ObjectImpl::get(selectedObject.value()));
        assert(pFrameBuffer && "Expected dynamic type match FrameBufferImpl");
        auto& frameBuffer = *pFrameBuffer;

        if(frameBuffer.renderPass.lock()->onscreen){
            ImGui::Text("Type: onscreen");

        } else{
            ImGui::Text("Type: offscreen");
            ImGui::Separator();
            ImGui::Text("Attachments: ");
            for(auto& imageMapped: frameBuffer.attachmentsImages){
                auto* image = &imageMapped.second;
                ImGui::Text("Attachment #%d: ", imageMapped.first);
                ImGui::SameLine();
                selectable(image->get());
            }
        }

    }

    void ObjectInspector::displayCameraInfo() {
        auto* pCamera = dynamic_cast<CameraImpl*>(ObjectImpl::get(selectedObject.value()));
        assert(pCamera && "Expected dynamic type match CameraImpl");
        auto& camera = *pCamera;

        ImGui::DragFloat3("Position", reinterpret_cast<float*>(&camera.position));
        ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&camera.rotation));
    }

    void ObjectInspector::displayLightInfo() {
        auto* pLight = dynamic_cast<LightImpl*>(ObjectImpl::get(selectedObject.value()));
        assert(pLight && "Expected dynamic type match LightImpl");
        auto& light = *pLight;


        ImGui::DragFloat2("Direction", reinterpret_cast<float*>(&light.direction));
        ImGui::ColorPicker3("Color", reinterpret_cast<float*>(&light.color));
    }

    void ObjectInspector::displayGeometryInfo() {
        auto* pGeometry = dynamic_cast<GeometryImpl*>(ObjectImpl::get(selectedObject.value()));
        assert(pGeometry && "Expected dynamic type match LightImpl");
        auto& geometry = *pGeometry;

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

            for (auto attr: geometry.vertexFormat.perVertexAttributes)
                iterate(attr);

            if (geometry.vertexFormat.perVertexAttributes.empty())
                ImGui::Text("empty");

            ImGui::Separator();

            ImGui::Text("Per-Instance:");

            for (auto attr: geometry.vertexFormat.perInstanceAttributes)
                iterate(attr);

            if (geometry.vertexFormat.perInstanceAttributes.empty())
                ImGui::Text("empty");

        }
        if (ImGui::CollapsingHeader("Descriptor layout")) {
            for (auto const &descriptor: geometry.descriptors) {
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
                ImGui::Separator();
            }
        }

    }

}