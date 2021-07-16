//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include "VulgineFramebuffer.h"
#include "Vulgine.h"
#include "Utilities.h"

void Vulgine::Framebuffer::createImpl() {
    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = renderPass;
    frameBufferCreateInfo.attachmentCount = attachments.size();
    frameBufferCreateInfo.pAttachments = attachments.data();
    frameBufferCreateInfo.width = width;
    frameBufferCreateInfo.height = height;
    frameBufferCreateInfo.layers = 1;

    VK_CHECK_RESULT(vkCreateFramebuffer(vlg_instance->device->logicalDevice, &frameBufferCreateInfo, nullptr, &framebuffer));

}

void Vulgine::Framebuffer::destroyImpl() {
    vkDestroyFramebuffer(vlg_instance->device->logicalDevice, framebuffer, nullptr);
}

Vulgine::Framebuffer::~Framebuffer() {
    if(isCreated())
        vkDestroyFramebuffer(vlg_instance->device->logicalDevice, framebuffer, nullptr);
}
