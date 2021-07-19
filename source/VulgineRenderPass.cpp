//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include <glm/vec4.hpp>
#include "VulgineRenderPass.h"
#include "Vulgine.h"
#include "Utilities.h"
#include "VulgineScene.h"
#include "vulkan/VulkanInitializers.hpp"


void Vulgine::RenderPassImpl::begin(VkCommandBuffer buffer, int currentFrame) {
    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = frameBuffer.width;
    renderPassBeginInfo.renderArea.extent.height = frameBuffer.height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.framebuffer = frameBuffer.framebuffers.at(currentFrame);

    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

}

void Vulgine::RenderPassImpl::end(VkCommandBuffer buffer) {
    vkCmdEndRenderPass(buffer);
}

void Vulgine::RenderPassImpl::buildCmdBuffers(VkCommandBuffer buffer, int currentFrame) {


    VkViewport viewport = initializers::viewport((float)frameBuffer.width, (float)frameBuffer.height, 0.0f, 1.0f);
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor = initializers::rect2D(frameBuffer.width, frameBuffer.height, 0, 0);
    vkCmdSetScissor(buffer, 0, 1, &scissor);

    //draw scene from perspective of specified camera

    dynamic_cast<SceneImpl*>(scene)->draw(buffer, dynamic_cast<CameraImpl*>(camera), this);

}

Vulgine::FrameBuffer *Vulgine::RenderPassImpl::getFrameBuffer() {
    return &frameBuffer;
}

Vulgine::RenderPassImpl::~RenderPassImpl() {
    if(isCreated())
        vkDestroyRenderPass(vlg_instance->device->logicalDevice, renderPass, nullptr);
}

void Vulgine::RenderPassImpl::createImpl() {
    // nothing here yet (still need to call create() to notify that VkCreateRenderPass happened)
}

void Vulgine::RenderPassImpl::destroyImpl() {
    vkDestroyRenderPass(vlg_instance->device->logicalDevice, renderPass, nullptr);
}
