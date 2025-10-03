#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <cstring>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

namespace Prometheus{
    class BufferManager{
    public:
        static void createFrameBuffers(VkDevice& device);
        static void createCommandPool(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device);
        static void createCommandBuffers(VkDevice& device);
        static void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex,
            VkDevice& device, VkPhysicalDevice& physicalDevice);

        static void createIndexVertexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);
        static void recreateVerIndBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);

        static uint32_t findMemoryType(uint32_t& typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice);

        static void createBuffer(VkDeviceSize size,
            VkBufferUsageFlags usage, 
            VkMemoryPropertyFlags properties, 
            VkBuffer& buffer, 
            VkDeviceMemory& bufferMemory,
            VkDevice& device,
            VkPhysicalDevice& physicalDevice
        );

        static void createJointBuffer(VkDeviceSize size1, 
            VkDeviceSize size2, 
            VkDeviceSize& offset1, 
            VkBufferUsageFlags usage, 
            VkMemoryPropertyFlags properties, 
            VkBuffer& buffer, 
            VkDeviceMemory& bufferMemory, 
            VkDevice& device, 
            VkPhysicalDevice& physicalDevice
        );

        static void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, VkDevice& device, VkQueue& graphicsQueue);

        static void createUniformBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice);
        static void updateUniformBuffer(uint32_t currentImage);

        static VkCommandBuffer beginSingleTimeCommands(VkDevice& device);
        static void endSingleTimeCommands(VkCommandBuffer& commandBuffer, VkDevice& device, VkQueue& graphicsQueue);

        static void createInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice);
        static void updateInstanceBuffer(uint32_t currentImage);
        static void recreateInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice);

        static void createDepthResources(VkDevice& device,VkPhysicalDevice& physicalDevice);
        static void createColorResources(VkDevice& device, VkPhysicalDevice& physicalDevice);

        static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features,
            VkPhysicalDevice& physicalDevice);
        static VkFormat findDepthFormat(VkPhysicalDevice& physicalDevice);
        static bool hasStencilComponent(VkFormat format);
    };
}