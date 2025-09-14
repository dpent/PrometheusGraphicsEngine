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
}