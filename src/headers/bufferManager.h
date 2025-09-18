#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

namespace Prometheus{
    class BufferManager{
    public:
        static void createFrameBuffers(VkDevice& device);
        static void createCommandPool(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device);
        static void createCommandBuffers(VkDevice& device);
        static void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex);

        static void createIndexVertexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);

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
    };
}