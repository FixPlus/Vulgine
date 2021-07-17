//
// Created by Бушев Дмитрий on 17.07.2021.
//

#include "VulgineObject.h"
#include "Utilities.h"


namespace Vulgine{

    std::unordered_map<Object::Type, uint32_t> ObjectImpl::countMap;
    std::unordered_map<Object::Type, std::string> ObjectImpl::typeNames;


    ObjectImpl::ObjectImpl(uint32_t id, Type typeId): id_(id), typeId_(typeId){
        logger(typeNames.at(typeId_) + " #" + std::to_string(id_) + " created");
        if(countMap.count(typeId))
            countMap.at(typeId)++;
        else
            countMap.emplace(typeId, 1);

    }

    ObjectImpl::~ObjectImpl() {
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

    uint32_t Object::count(Type type) { return ObjectImpl::count(type);}
}