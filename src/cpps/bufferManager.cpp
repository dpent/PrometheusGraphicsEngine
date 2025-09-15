#include "../headers/bufferManager.h"
#include "../headers/engine.h"
#include "../headers/queueManager.h"


using namespace Prometheus;

namespace Prometheus{
    void BufferManager::createFrameBuffers(VkDevice& device){
        Engine::swapChainFramebuffers.resize(Engine::swapChainImageViews.size());

        for (size_t i = 0; i < Engine::swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                Engine::swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = Engine::renderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = Engine::swapChainExtent.width;
            framebufferInfo.height = Engine::swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &Engine::swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void BufferManager::createCommandPool(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device){
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &Engine::commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void BufferManager::createCommandBuffer(VkDevice& device){
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = Engine::commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;/*
                                                            -- VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution,
                                                            but cannot be called from other command buffers.
                                                            -- VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can
                                                            be called from primary command buffers.
                                                        */
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(device, &allocInfo, &Engine::commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void BufferManager::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex){

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional 
                                /*
                                -- VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
                                -- VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
                                -- VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
                                */
        beginInfo.pInheritanceInfo = nullptr; // Optional
        

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = Engine::renderPass;
        renderPassInfo.framebuffer = Engine::swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = Engine::swapChainExtent;

        VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; //This is the background color or when nothing exists there it paints it this color
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        /* 
        -- VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
        -- VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
        */

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Engine::graphicsPipeline);

        VkViewport viewport{}; //Viewport and Scissor was set to dynamic in createGraphicsPipeline (graphicsPipeline.cpp)
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(Engine::swapChainExtent.width);
        viewport.height = static_cast<float>(Engine::swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = Engine::swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdDraw(commandBuffer, 3, 1, 0, 0); //Draws

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

    }
}