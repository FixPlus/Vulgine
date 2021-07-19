//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineFramebuffer.h"
#include "Vulgine.h"
#include "Utilities.h"
#include "VulgineRenderPass.h"

void Vulgine::FrameBufferImpl::createImpl() {

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass->renderPass;
    framebuffers.resize(vlg_instance->swapChain.imageCount);
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    for(int i = 0; i < vlg_instance->swapChain.imageCount; i++) {

        frameBufferCreateInfo.attachmentCount = attachments.at(i).size();
        frameBufferCreateInfo.pAttachments = attachments.at(i).data();
        VK_CHECK_RESULT(vkCreateFramebuffer(vlg_instance->device->logicalDevice, &frameBufferCreateInfo, nullptr,
                                            &framebuffers[i]));
    }

}

void Vulgine::FrameBufferImpl::destroyImpl() {
    for(auto framebuffer: framebuffers)
        vkDestroyFramebuffer(vlg_instance->device->logicalDevice, framebuffer, nullptr);

    if(!renderPass->onscreen)
        for(auto& attachmentFrame: attachments)
            for(auto attachment: attachmentFrame)
                vkDestroyImageView(vlg_instance->device->logicalDevice, attachment, nullptr);

    attachments.clear();
    attachmentsImages.clear();

}

Vulgine::FrameBufferImpl::~FrameBufferImpl() {
    if(isCreated()) {
        for (auto framebuffer: framebuffers)
            vkDestroyFramebuffer(vlg_instance->device->logicalDevice, framebuffer, nullptr);

        if(!renderPass->onscreen)
            for(auto& attachmentFrame: attachments)
                for(auto attachment: attachmentFrame)
                    vkDestroyImageView(vlg_instance->device->logicalDevice, attachment, nullptr);
    }
}

Vulgine::Image *Vulgine::FrameBufferImpl::addAttachment(Type type) {
    attachments.resize(vlg_instance->swapChain.imageCount);

    if(type == FrameBuffer::Type::DEPTH_STENCIL){
        if(std::any_of(attachmentsImages.begin(), attachmentsImages.end(),
                       [](std::pair<const uint32_t , DynamicImageImpl> const& image){
            return image.second.images.at(0).imageInfo.format == vlg_instance->device->getSupportedDepthFormat(true);})){
            errs("Cannot bind more than one depth attachment to framebuffer");
            return nullptr;
        }
    }
    auto binding = attachments.at(0).size();

    auto& image = attachmentsImages.emplace(std::piecewise_construct, std::forward_as_tuple(binding),
                                            std::forward_as_tuple(ObjectImpl::claimId())).first->second;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    if(type == Type::COLOR)
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB; // TODO: is it right format for framebuffer?...
    else
        imageInfo.format = vlg_instance->device->getSupportedDepthFormat(true);

    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if(type == Type::COLOR)
        imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: are we gonna always sample from it?
    else
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; // TODO: are we gonna always sample from it?

    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: make handle for multisampling support
    imageInfo.flags = 0; // Optional

    image.createInfo = imageInfo;
    image.create();

    // create on view per every swap chain image
    for(int i = 0; i < vlg_instance->swapChain.imageCount; ++i) {
        VkImageView view;
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = imageInfo.format;
        viewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                     VK_COMPONENT_SWIZZLE_A};

        // TODO: also make handle for stencil view aspect

        viewCreateInfo.subresourceRange = {static_cast<VkImageAspectFlags>((type == Type::COLOR) ? VK_IMAGE_ASPECT_COLOR_BIT : (imageInfo.format >= VK_FORMAT_D16_UNORM_S8_UINT) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_DEPTH_BIT), 0, 1, 0, 1};
        // Linear tiling usually won't support mip maps
        // Only set mip map count if optimal tiling is used
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.image = image.images.at(i).image;
        VK_CHECK_RESULT(vkCreateImageView(vlg_instance->device->logicalDevice, &viewCreateInfo, nullptr, &view));
        attachments.at(i).push_back(view);
    }

    return &image;
}

Vulgine::Image *Vulgine::FrameBufferImpl::getAttachment(uint32_t binding) {
    auto it = attachmentsImages.find(binding);

    if(it != attachmentsImages.end()) {
        return &it->second;
    }

    return nullptr;
}

uint32_t Vulgine::FrameBufferImpl::attachmentCount() {
    return attachmentsImages.size();
}

void Vulgine::FrameBufferImpl::addOnsrceenAttachment(std::vector<VkImageView> const& views) {
    assert(views.size() == vlg_instance->swapChain.imageCount && "Must provide one view per swapChain image");

    if(attachments.empty())
        attachments.resize(vlg_instance->swapChain.imageCount);

    for(int i = 0; i < vlg_instance->swapChain.imageCount; i++)
        attachments.at(i).push_back(views.at(i));
}

