//
// Created by Бушев Дмитрий on 17.07.2021.
//

#include "VulgineObjects.h"
#include "Vulgine.h"
#include "Utilities.h"


namespace Vulgine{

    std::unordered_map<Object::Type, uint32_t> ObjectImpl::countMap{};
    std::unordered_map<Object::Type, std::string> ObjectImpl::typeNames{};

    std::stack<uint32_t> ObjectImpl::freeIds{};
    uint32_t ObjectImpl::claimedIdsCount = 0;
    std::unordered_map<uint32_t, ObjectImpl*> ObjectImpl::objMap{};

    ObjectImpl::ObjectImpl(Type typeId, uint32_t id): typeId_(typeId), id_(id){

        objMap.emplace(id_, this);

        logger(typeNames.at(typeId_) + " #" + std::to_string(id_) + " created");
        if(countMap.count(typeId))
            countMap.at(typeId)++;
        else
            countMap.emplace(typeId, 1);

    }

    ObjectImpl::~ObjectImpl() {
        if(id_ == UINT32_MAX)
            return;
        invalidateId(id_);

        if(name.has_value())
            logger(name.value() +  "("+ typeNames.at(typeId_) + " #" + std::to_string(id_) + ") destroyed");
        else
            logger(typeNames.at(typeId_) + " #" + std::to_string(id_) + " destroyed");
        countMap.at(typeId_)--;
    };

uint32_t ObjectImpl::count(Type type) { return countMap.count(type) ? countMap.at(type) : 0;}

    void ObjectImpl::fillTypeNameTable() {
        typeNames.emplace(Type::MATERIAL,      "Material");
        typeNames.emplace(Type::SCENE,         "Scene");
        typeNames.emplace(Type::UBO,           "Uniform Buffer");
        typeNames.emplace(Type::IMAGE,         "Image");
        typeNames.emplace(Type::MESH,          "Mesh");
        typeNames.emplace(Type::CAMERA,        "Camera");
        typeNames.emplace(Type::LIGHT,         "Light");
        typeNames.emplace(Type::SHADER_MODULE, "Shader Module");
        typeNames.emplace(Type::RENDER_PASS,   "Render pass");
        typeNames.emplace(Type::FRAME_BUFFER,  "Framebuffer");
        typeNames.emplace(Type::PIPELINE,      "Pipeline");
        typeNames.emplace(Type::UNKNOWN,       "Unknown object");
    }

    std::string ObjectImpl::objectLabel() const {
        if(name.has_value()){
            return name.value();
        } else {
            return typeNames.at(typeId_) + " #" + std::to_string(id_);
        }
    }

    std::string ObjectImpl::typeName() const {
        return typeNames.at(typeId_);
    }

    uint32_t ObjectImpl::claimId() {
        claimedIdsCount++;
        if(freeIds.empty()){
            return claimedIdsCount - 1;
        } else{
            auto ret = freeIds.top();
            freeIds.pop();
            return ret;
        }
    }

    void ObjectImpl::invalidateId(uint32_t id) {
        freeIds.push(id);
        objMap.erase(id);
        claimedIdsCount--;
    }

    ObjectImpl *ObjectImpl::get(uint32_t id) {
        auto it = objMap.find(id);

        if(it != objMap.end())
            return it->second;
        else
            return nullptr;
    }

    ObjectImpl &ObjectImpl::operator=(ObjectImpl &&another) noexcept{
        id_ = another.id_;
        typeId_ = another.typeId_;
        name = another.name;
        created = another.created;

        objMap.at(id_) = this;
        another.id_ = UINT32_MAX;
        another.created = false;

        return *this;
    }

    ObjectImpl::ObjectImpl(ObjectImpl &&another) noexcept: id_(another.id_), name(another.name),
    typeId_(another.typeId_){
        created = another.created;
        objMap.at(id_) = this;
        another.id_ = UINT32_MAX;
        another.created = false;
    }

    void ObjectImpl::for_each(std::function<void(ObjectImpl *)> action) {
        for(auto it: objMap)
            action(it.second);
    }

    Object* Object::get(uint32_t id){
        return ObjectImpl::get(id);
    }
    uint32_t Object::count(Type type) { return ObjectImpl::count(type);}

    bool checkDeviceLimits(ObjectImpl* object){
        if(auto* renderPass = dynamic_cast<RenderPassImpl*>(object)){
            if(renderPass->onscreen)
                return true;
            auto maxColorAttachments = vlg_instance->device->properties.limits.maxColorAttachments;
            uint32_t colorAttachmentCount = 0;
            for(auto const& attachment: renderPass->frameBuffer.attachmentsImages){
                if(attachment.second.createInfo.format != vlg_instance->depthFormat)
                    colorAttachmentCount++;
            }

            if(colorAttachmentCount > maxColorAttachments){
                errs(renderPass->objectLabel() + " exceeds device.limits.maxColorAttachments("
                + std::to_string(maxColorAttachments) + ") by having " + std::to_string(colorAttachmentCount) + " of them");
                return false;
            }
        }

        if(auto* mesh = dynamic_cast<MeshImpl*>(object)){
            auto maxVertexInputAttachments = vlg_instance->device->properties.limits.maxVertexInputAttributes;
            auto vertexInputAttachments = mesh->vertexStageInfo.vertexFormat.perInstanceAttributes.size() +
                                          mesh->vertexStageInfo.vertexFormat.perVertexAttributes.size();

            if(vertexInputAttachments > maxVertexInputAttachments){
                errs(mesh->objectLabel() + " exceeds device.limits.maxVertexInputAttributes(" + std::to_string(maxVertexInputAttachments) +
                ") by having" + std::to_string(vertexInputAttachments) + " of them");
                return false;
            }
        }

        return true;
    }
}