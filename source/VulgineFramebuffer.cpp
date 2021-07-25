//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineFramebuffer.h"
#include "Vulgine.h"
#include "Utilities.h"
#include "VulgineRenderPass.h"

void Vulgine::FrameBufferImpl::createImpl() {

    auto const& renderPassRef = *(renderPass.lock().get());

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPassRef.renderPass;
    framebuffers.resize(GetImpl().swapChain.imageCount);
    frameBufferCreateInfo.width = renderPassRef.framebufferExtents.width;
    frameBufferCreateInfo.height = renderPassRef.framebufferExtents.height;
    frameBufferCreateInfo.layers = 1;

    for(int i = 0; i < GetImpl().swapChain.imageCount; i++) {

        frameBufferCreateInfo.attachmentCount = attachments.at(i).size();
        frameBufferCreateInfo.pAttachments = attachments.at(i).data();
        VK_CHECK_RESULT(vkCreateFramebuffer(GetImpl().device->logicalDevice, &frameBufferCreateInfo, nullptr,
                                            &framebuffers[i]));
    }

}

void Vulgine::FrameBufferImpl::destroyImpl() {
    for(auto framebuffer: framebuffers)
        vkDestroyFramebuffer(GetImpl().device->logicalDevice, framebuffer, nullptr);

    for(auto& attachmentFrame: attachments)
        for(int i = 0; i < attachmentFrame.size(); i++)
            if(attachmentsImages.count(i))
                vkDestroyImageView(GetImpl().device->logicalDevice, attachmentFrame.at(i), nullptr);


    attachments.clear();
    attachmentsImages.clear();
    hasDepth = false;
}

Vulgine::FrameBufferImpl::~FrameBufferImpl() {
    if(isCreated()) {
        for (auto framebuffer: framebuffers)
            vkDestroyFramebuffer(GetImpl().device->logicalDevice, framebuffer, nullptr);

        for(auto& attachmentFrame: attachments)
            for(int i = 0; i < attachmentFrame.size(); i++)
                if(attachmentsImages.count(i))
                    vkDestroyImageView(GetImpl().device->logicalDevice, attachmentFrame.at(i), nullptr);
    }
}

Vulgine::ImageRef Vulgine::FrameBufferImpl::addAttachment(RenderPass::AttachmentType type) {
    if(type == RenderPass::AttachmentType::DEPTH_STENCIL && hasDepth){
        errs("Cannot bind more than one depth attachment to framebuffer");
        return nullptr;
    }
    auto format = type == RenderPass::AttachmentType::COLOR ? ColorBufferFormat : GetImpl().device->getSupportedDepthFormat(true);;
    auto usage = type == RenderPass::AttachmentType::COLOR ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT:
                 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    return createAttachment(format, usage);
}

Vulgine::ImageRef Vulgine::FrameBufferImpl::getAttachment(uint32_t binding) {
    auto it = attachmentsImages.find(binding);

    if(it != attachmentsImages.end()) {
        return it->second;
    }

    return nullptr;
}

uint32_t Vulgine::FrameBufferImpl::attachmentCount() {
    return attachmentsImages.size();
}

void Vulgine::FrameBufferImpl::addOnsrceenAttachment(std::vector<VkImageView> const& views) {
    assert(views.size() == GetImpl().swapChain.imageCount && "Must provide one view per swapChain image");

    if(attachments.empty())
        attachments.resize(GetImpl().swapChain.imageCount);

    for(int i = 0; i < GetImpl().swapChain.imageCount; i++)
        attachments.at(i).push_back(views.at(i));
}

Vulgine::ImageRef Vulgine::FrameBufferImpl::createAttachment(VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples) {
    attachments.resize(GetImpl().swapChain.imageCount);

    auto const& renderPassRef = *(renderPass.lock().get());

    auto binding = attachments.at(0).size();

    auto& image = attachmentsImages.emplace(std::piecewise_construct, std::forward_as_tuple(binding),
                                            std::forward_as_tuple(new DynamicImageImpl{ObjectImpl::claimId()})).first->second;

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(renderPassRef.framebufferExtents.width);
    imageInfo.extent.height = static_cast<uint32_t>(renderPassRef.framebufferExtents.height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.format = format;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = samples;
    imageInfo.flags = 0; // Optional

    image->createInfo = imageInfo;
    image->create();
    VkImageAspectFlags aspectMask = 0;

    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        hasDepth = true;
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    // create on view per every swap chain image
    for(int i = 0; i < GetImpl().swapChain.imageCount; ++i) {
        VkImageView view;
        VkImageViewCreateInfo viewCreateInfo = {};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.format = imageInfo.format;
        viewCreateInfo.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
                                     VK_COMPONENT_SWIZZLE_A};

        // TODO: also make handle for stencil view aspect

        viewCreateInfo.subresourceRange = {aspectMask, 0, 1, 0, 1};
        // Linear tiling usually won't support mip maps
        // Only set mip map count if optimal tiling is used
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.image = image->images.at(i).image;
        VK_CHECK_RESULT(vkCreateImageView(GetImpl().device->logicalDevice, &viewCreateInfo, nullptr, &view));
        attachments.at(i).push_back(view);
    }

    return image;

}

void Vulgine::FrameBufferImpl::createGBuffer() {

    createAttachment(GBufferPosFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);	// (World space) Positions
    createAttachment(GBufferNormFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);		// (World space) Normals
    createAttachment(GBufferAlbedoFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);			// Albedo (color)

}

uint32_t Vulgine::FrameBufferImpl::colorAttachmentCount() {
    return hasDepth ? attachments.at(0).size() - 1 : attachments.at(0).size();
}

