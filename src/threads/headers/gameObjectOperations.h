#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>


namespace Prometheus
{
    void createObject(std::string texturePath, std::string modelPath, 
        VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);

    void deleteObject(uint64_t id,VkDevice& device);

    void updateTextureDeleteQueue(VkDevice& device);
}
