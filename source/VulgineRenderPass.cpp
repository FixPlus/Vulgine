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

    VkRenderPassBeginInfo renderPassBeginInfo = initializers::renderPassBeginInfo();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = frameBuffer.width;
    renderPassBeginInfo.renderArea.extent.height = frameBuffer.height;
    renderPassBeginInfo.clearValueCount = clearValues.size();
    renderPassBeginInfo.pClearValues = clearValues.data();
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

    dynamic_cast<SceneImpl*>(scene)->draw(buffer, dynamic_cast<CameraImpl*>(camera), this, currentFrame);

    if(!deferredEnabled)
        dynamic_cast<SceneImpl*>(scene)->drawBackground(buffer, dynamic_cast<CameraImpl*>(camera), this, currentFrame);

    if(deferredEnabled){
        vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE);
        deferredLightingSubpass.pipeline.bind(buffer);
        deferredLightingSubpass.compositionSet.bind(0, buffer,
                                                    deferredLightingSubpass.pipeline.pipelineLayout,
                                                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    currentFrame);

        auto& cameraImpl = *dynamic_cast<CameraImpl*>(camera);
        vkCmdPushConstants(buffer, deferredLightingSubpass.pipeline.pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(cameraImpl.matrices), &(cameraImpl.matrices));
        vkCmdDraw(buffer, 3, 1, 0, 0);

        vkCmdNextSubpass(buffer, VK_SUBPASS_CONTENTS_INLINE);

        dynamic_cast<SceneImpl*>(scene)->drawBackground(buffer, dynamic_cast<CameraImpl*>(camera), this, currentFrame);
        // TODO: invoke transparent material draw list in scene
    }

}

Vulgine::FrameBuffer *Vulgine::RenderPassImpl::getFrameBuffer() {
    return &frameBuffer;
}

Vulgine::RenderPassImpl::~RenderPassImpl() {
    if(isCreated())
        vkDestroyRenderPass(GetImpl().device->logicalDevice, renderPass, nullptr);
}

void Vulgine::RenderPassImpl::createImpl() {
    // nothing here yet (still need to call create() to notify that VkCreateRenderPass happened)
}

void Vulgine::RenderPassImpl::destroyImpl() {
    vkDestroyRenderPass(GetImpl().device->logicalDevice, renderPass, nullptr);
}

void Vulgine::RenderPassImpl::buildPass() {
    destroy();

    clearValues.clear();

    createFramebuffer();

    CHECK_DEVICE_LIMITS(this)


    VkFormat colorTargetFormat = onscreen ? GetImpl().swapChain.colorFormat : FrameBufferImpl::ColorBufferFormat;

    VkFormat depthFormat = GetImpl().device->getSupportedDepthFormat(true);

    deferredEnabled = dynamic_cast<SceneImpl*>(scene)->hasDynamicLights();

    if(deferredEnabled){
        std::array<VkAttachmentDescription, 5> attachments = {};

        // Color attachment
        attachments[0].format = colorTargetFormat;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = onscreen ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachments[0].finalLayout = !onscreen ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL :
                                     VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // G-Buffer position
        attachments[1].format = FrameBufferImpl::GBufferPosFormat;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // G-Buffer normal
        attachments[2].format = FrameBufferImpl::GBufferNormFormat;
        attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // G-Buffer albedo
        attachments[3].format = FrameBufferImpl::GBufferAlbedoFormat;
        attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // Depth attachment
        attachments[4].format = depthFormat;
        attachments[4].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[4].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[4].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[4].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[4].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[4].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[4].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Three subpasses
        std::array<VkSubpassDescription,3> subpassDescriptions{};

        // First subpass: Fill G-Buffer components
        // ----------------------------------------------------------------------------------------

        VkAttachmentReference colorReferences[4];
        colorReferences[0] = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        colorReferences[1] = { 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        colorReferences[2] = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        colorReferences[3] = { 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depthReference = { 4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

        subpassDescriptions[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[0].colorAttachmentCount = 4;
        subpassDescriptions[0].pColorAttachments = colorReferences;
        subpassDescriptions[0].pDepthStencilAttachment = &depthReference;

        // Second subpass: Final composition (using G-Buffer components)
        // ----------------------------------------------------------------------------------------

        VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        VkAttachmentReference inputReferences[3];
        inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        inputReferences[1] = { 2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        inputReferences[2] = { 3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

        uint32_t preserveAttachmentIndex = 1;

        subpassDescriptions[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[1].colorAttachmentCount = 1;
        subpassDescriptions[1].pColorAttachments = &colorReference;
        subpassDescriptions[1].pDepthStencilAttachment = &depthReference;
        // Use the color attachments filled in the first pass as input attachments
        subpassDescriptions[1].inputAttachmentCount = 3;
        subpassDescriptions[1].pInputAttachments = inputReferences;

        // Third subpass: Forward transparency
        // ----------------------------------------------------------------------------------------
        colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

        inputReferences[0] = { 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

        subpassDescriptions[2].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescriptions[2].colorAttachmentCount = 1;
        subpassDescriptions[2].pColorAttachments = &colorReference;
        subpassDescriptions[2].pDepthStencilAttachment = &depthReference;
        // Use the color/depth attachments filled in the first pass as input attachments
        subpassDescriptions[2].inputAttachmentCount = 1;
        subpassDescriptions[2].pInputAttachments = inputReferences;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 4> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // This dependency transitions the input attachment from color attachment to shader read
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = 1;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[2].srcSubpass = 1;
        dependencies[2].dstSubpass = 2;
        dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[2].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[3].srcSubpass = 0;
        dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[3].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescriptions.size());
        renderPassInfo.pSubpasses = subpassDescriptions.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        VK_CHECK_RESULT(vkCreateRenderPass(GetImpl().device->logicalDevice, &renderPassInfo, nullptr, &renderPass));


        // Now must setup descriptors



        deferredLightingSubpass.pipeline.renderPass = this;
        deferredLightingSubpass.pipeline.create();

        clearValues.resize(5);
        for(int i = 0; i < 4; i++)
            clearValues.at(i).color = {0.025f, 0.025f, 0.025f, 1.0f};

        clearValues.at(4).depthStencil = {1.0f, 0};

    } else {
        if(onscreen){

            // if no dynamic lights present - use simple forward rendering technic

            std::array<VkAttachmentDescription, 3> attachments = {};

            // Color attachment
            attachments[0].format = colorTargetFormat;
            attachments[0].samples = GetImpl().settings.msaa;
            attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[0].finalLayout = GetImpl().settings.msaa > 1 ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL :
                                         VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            // Depth attachment
            attachments[1].format = depthFormat;
            attachments[1].samples = GetImpl().settings.msaa;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            // Reslove color attachment for multisampling image

            if (GetImpl().settings.msaa > 1) {
                attachments[2].format = GetImpl().swapChain.colorFormat;
                attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
                attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachments[2].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            }

            VkAttachmentReference colorReference = {};
            colorReference.attachment = 0;
            colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthReference = {};
            depthReference.attachment = 1;
            depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentResolveRef{};
            colorAttachmentResolveRef.attachment = 2;
            colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = 1;
            subpassDescription.pColorAttachments = &colorReference;
            subpassDescription.pDepthStencilAttachment = &depthReference;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = nullptr;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = nullptr;
            subpassDescription.pResolveAttachments = GetImpl().settings.msaa > 1 ? &colorAttachmentResolveRef : nullptr;

            // Subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies{};

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(GetImpl().settings.msaa > 1 ? attachments.size() :
                                                                   attachments.size() - 1);
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VK_CHECK_RESULT(
                    vkCreateRenderPass(GetImpl().device->logicalDevice, &renderPassInfo, nullptr, &renderPass));


            clearValues.resize(2);

            clearValues.at(0).color = {0.025f, 0.025f, 0.025f, 1.0f};
            clearValues.at(1).depthStencil = {1.0f, 0};

        } else{

            // fully custom offscreen render pass

            if(frameBuffer.attachmentsImages.empty()){
                Utilities::ExitFatal(-1, "Cannot create render pass with empty attachments");
            }

            std::vector<VkAttachmentDescription> attachments = {};
            std::optional<VkAttachmentReference> depthAttachemntRef;
            std::vector<VkAttachmentReference> colorAttachmentRefs{};
            attachments.resize(frameBuffer.attachmentCount());

            for(auto& it: frameBuffer.attachmentsImages) {
                auto binding = it.first;
                auto& image = it.second;


                attachments.at(binding).format = image.createInfo.format;
                attachments.at(binding).samples = VK_SAMPLE_COUNT_1_BIT; // no multisampling for offscreen rendering yet
                attachments.at(binding).loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                attachments.at(binding).storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                attachments.at(binding).stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                attachments.at(binding).stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

                // for now we consider it to be shader read-only optimal

                attachments.at(binding).initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                // for now we consider it to be used as sampled image

                attachments.at(binding).finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                if(image.createInfo.format == depthFormat){
                    depthAttachemntRef.emplace();
                    depthAttachemntRef->attachment = binding;
                    depthAttachemntRef->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                } else{
                    auto& colRef = colorAttachmentRefs.emplace_back();
                    colRef.attachment = binding;
                    colRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }

                clearValues.emplace_back();
                if(image.createInfo.format == FrameBufferImpl::ColorBufferFormat)
                    clearValues.back().color = {0.0f, 0.0f, 0.0f, 0.0f};
                else
                    clearValues.back().depthStencil = {0.0f, 0};
            }


            VkSubpassDescription subpassDescription = {};
            subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.colorAttachmentCount = colorAttachmentRefs.size();
            subpassDescription.pColorAttachments = colorAttachmentRefs.data();
            subpassDescription.pDepthStencilAttachment = depthAttachemntRef.has_value() ? &depthAttachemntRef.value() : nullptr;
            subpassDescription.inputAttachmentCount = 0;
            subpassDescription.pInputAttachments = nullptr;
            subpassDescription.preserveAttachmentCount = 0;
            subpassDescription.pPreserveAttachments = nullptr;
            subpassDescription.pResolveAttachments = nullptr;

            // Subpass dependencies for layout transitions
            std::array<VkSubpassDependency, 2> dependencies{};

            // TODO: deduct external dependencies by framebuffer images use info

            dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[0].dstSubpass = 0;
            dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            dependencies[1].srcSubpass = 0;
            dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
            dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependencies[1].dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
            dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

            VkRenderPassCreateInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = attachments.size();

            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpassDescription;
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            VK_CHECK_RESULT(
                    vkCreateRenderPass(GetImpl().device->logicalDevice, &renderPassInfo, nullptr, &renderPass))
        }

    }

    frameBuffer.renderPass = this;
    frameBuffer.create();

}

void Vulgine::RenderPassImpl::createFramebuffer() {
    assert(!onscreen || (onscreen && frameBuffer.attachments.empty()) && "Cant override onscreen framebuffer");
    bool deferred = dynamic_cast<SceneImpl*>(scene)->hasDynamicLights();
    //assert(!deferred || (deferred && frameBuffer.attachments.empty()) && "Framebuffer must stay unmodified when using dynamic lights");

    bool mainColorTargetAllocated = false;
    if(!frameBuffer.attachments.empty() && deferred){
        assert(frameBuffer.colorAttachmentCount() == 1 && frameBuffer.attachmentCount() == 1 && "Can only preallocate main color target");
        mainColorTargetAllocated = true;
    }
    if(onscreen ||  deferred){
        if(onscreen){

            frameBuffer.width = GetImpl().vieportInfo.width;
            frameBuffer.height = GetImpl().vieportInfo.height;

            if(!deferred)
            {
                if(GetImpl().settings.msaa > 1) { // using MSAA here
                    frameBuffer.createAttachment(GetImpl().swapChain.colorFormat,
                                                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, GetImpl().settings.msaa);
                    frameBuffer.addAttachment(FrameBuffer::Type::DEPTH_STENCIL);


                    std::vector<VkImageView> views;
                    for (int i = 0; i < GetImpl().swapChain.imageCount; i++) {
                        views.push_back(GetImpl().swapChain.buffers[i].view);
                    }
                    frameBuffer.addOnsrceenAttachment(views);
                } else {
                    std::vector<VkImageView> views;
                    for (int i = 0; i < GetImpl().swapChain.imageCount; i++) {
                        views.push_back(GetImpl().swapChain.buffers[i].view);
                    }
                    frameBuffer.addOnsrceenAttachment(views);

                    frameBuffer.addAttachment(FrameBuffer::Type::DEPTH_STENCIL);
                }
                return;
            }
            std::vector<VkImageView> views;
            for (int i = 0; i < GetImpl().swapChain.imageCount; i++) {
                views.push_back(GetImpl().swapChain.buffers[i].view);
            }
            frameBuffer.addOnsrceenAttachment(views);
        } else{
            if(!mainColorTargetAllocated)
                frameBuffer.addAttachment(FrameBuffer::Type::COLOR);
        }

        if(deferred)
            frameBuffer.createGBuffer();

        frameBuffer.addAttachment(FrameBuffer::Type::DEPTH_STENCIL);
    }


    if(deferred){

        deferredLightingSubpass.compositionSet.clearDescriptors();
        deferredLightingSubpass.transparentSet.clearDescriptors();

        deferredLightingSubpass.compositionSet.destroy();
        deferredLightingSubpass.transparentSet.destroy();
        deferredLightingSubpass.compositionSet.pool = &GetImpl().perRenderPassPool;

        std::vector<VkImageView> views1, views2, views3;
        views1.reserve(GetImpl().swapChain.imageCount);
        views2.reserve(GetImpl().swapChain.imageCount);
        views3.reserve(GetImpl().swapChain.imageCount);

        for(int i = 0; i < GetImpl().swapChain.imageCount; i++){
            views1.push_back(frameBuffer.attachments.at(i).at(1));
            views2.push_back(frameBuffer.attachments.at(i).at(2));
            views3.push_back(frameBuffer.attachments.at(i).at(3));
        }
        deferredLightingSubpass.compositionSet.addInputAttachment(views1, VK_SHADER_STAGE_FRAGMENT_BIT);
        deferredLightingSubpass.compositionSet.addInputAttachment(views2, VK_SHADER_STAGE_FRAGMENT_BIT);
        deferredLightingSubpass.compositionSet.addInputAttachment(views3, VK_SHADER_STAGE_FRAGMENT_BIT);
        deferredLightingSubpass.compositionSet.addUniformBuffer(&dynamic_cast<SceneImpl*>(scene)->lightUBO, VK_SHADER_STAGE_FRAGMENT_BIT);
        deferredLightingSubpass.compositionSet.create();


        deferredLightingSubpass.transparentSet.pool = &GetImpl().perRenderPassPool;

        deferredLightingSubpass.transparentSet.addInputAttachment(views1, VK_SHADER_STAGE_FRAGMENT_BIT);
        deferredLightingSubpass.transparentSet.addUniformBuffer(&dynamic_cast<SceneImpl*>(scene)->lightUBO, VK_SHADER_STAGE_FRAGMENT_BIT);
        deferredLightingSubpass.transparentSet.create();

    }
}

void Vulgine::RenderPassImpl::destroyFramebuffer() {
    frameBuffer.destroy();
}
