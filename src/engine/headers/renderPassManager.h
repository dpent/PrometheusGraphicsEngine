#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    class RenderPassManager{
    public:
        static void createRenderPass(VkDevice& device, VkPhysicalDevice& physicalDevice);
    };
}