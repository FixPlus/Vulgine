//
// Created by Бушев Дмитрий on 04.07.2021.
//

#include <glm/vec4.hpp>
#include "VulgineRenderPass.h"
#include "Vulgine.h"
#include "Utilities.h"
#include "VulgineScene.h"
#include "vulkan/VulkanInitializers.hpp"

Vulgine::RenderPass::~RenderPass() {
    vkDestroyRenderPass(vlg_instance->device->logicalDevice, renderPass, nullptr);
}

void Vulgine::DefaultRenderPass::buildCmdBuffers(VkCommandBuffer buffer, Framebuffer *framebuffer) {

    VkClearValue clearValues[2];
    clearValues[0].color = { { 0.025f, 0.025f, 0.025f, 1.0f } };
    clearValues[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo renderPassBeginInfo = initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = framebuffer->width;
    renderPassBeginInfo.renderArea.extent.height = framebuffer->height;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;
    renderPassBeginInfo.framebuffer = framebuffer->framebuffer;

    vkCmdBeginRenderPass(buffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = initializers::viewport((float)framebuffer->width, (float)framebuffer->height, 0.0f, 1.0f);
    vkCmdSetViewport(buffer, 0, 1, &viewport);

    VkRect2D scissor = initializers::rect2D(framebuffer->width, framebuffer->height, 0, 0);
    vkCmdSetScissor(buffer, 0, 1, &scissor);


    //draw scene from perspective of specified camera

    scene->draw(buffer, camera, this);


    vkCmdEndRenderPass(buffer);

}
