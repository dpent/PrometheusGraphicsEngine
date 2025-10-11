#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../../engine/headers/bufferManager.h"
#include "../../engine/headers/engine.h"

namespace Prometheus{

    void updateVertexIndexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, 
        VkQueue& graphicsQueue,sem_t* jobDoneSem);

    void recreateInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice,
        sem_t* jobDoneSem);

    void updateInstanceBuffer(uint32_t currentImage);
}