#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Prometheus{
    class BufferManager{
    public:
        static void createFrameBuffers(VkDevice& device);
        static void createCommandPool(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device);
        static void createCommandBuffer(VkDevice& device);
        static void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex);
    };
}