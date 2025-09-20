#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Prometheus{
    class TextureManager{
    public:
        static void createTextureImage(const char * filename, int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkImage& image, VkDeviceMemory& imageMemory, VkQueue& graphicsQueue);

        static void createImage(uint32_t width, uint32_t height, VkFormat format, 
            VkImageTiling tiling, VkImageUsageFlags usage, 
            VkMemoryPropertyFlags properties, VkImage& image, 
            VkDeviceMemory& imageMemory,
            VkDevice& device,
            VkPhysicalDevice& physicalDevice);
        
        static void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
        VkDevice& device, VkQueue& graphicsQueue);

        static void copyBufferToImage(VkBuffer& buffer, VkImage& image, const uint32_t& width, const uint32_t& height,
        VkDevice& device, VkQueue& graphicsQueue);
    };
}