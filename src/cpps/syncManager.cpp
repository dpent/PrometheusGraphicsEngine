#include "../headers/syncManager.h"
#include "../headers/engine.h"


using namespace Prometheus;

namespace Prometheus{
    void SyncManager::createSyncObjects(VkDevice& device){
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &Engine::imageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &Engine::renderFinishedSemaphore) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &Engine::inFlightFence) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}