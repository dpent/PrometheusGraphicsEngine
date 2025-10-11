#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../../engine/headers/bufferManager.h"
#include "../../engine/headers/engine.h"

namespace Prometheus{

    void updateVertexIndexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, 
        VkQueue& graphicsQueue, VkCommandPool& commandPool);

    void recreateInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice,
        sem_t* jobDoneSem);

    void updateInstanceBuffer(uint32_t currentImage);

    void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex,
        VkDevice& device, VkPhysicalDevice& physicalDevice);
    
    void cleanup(VkDevice& device, VkCommandPool& commandPool);
}