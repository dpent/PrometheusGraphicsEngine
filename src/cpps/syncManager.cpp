#include "../headers/syncManager.h"
#include "../headers/engine.h"


using namespace Prometheus;

namespace Prometheus{
    void SyncManager::createSyncObjects(VkDevice& device){
        Engine::imageAvailableSemaphores.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::renderFinishedSemaphores.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::inFlightFences.resize(Engine::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;


        for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &Engine::imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(device, &semaphoreInfo, nullptr, &Engine::renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(device, &fenceInfo, nullptr, &Engine::inFlightFences[i]) != VK_SUCCESS) {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }
}