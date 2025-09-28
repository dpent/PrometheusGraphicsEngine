#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

namespace Prometheus{
    class TextureManager{
    public:
        static void createTextureImage(std::string filename, int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice,
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

        static void createTextureImageView(VkDevice& device, VkImage& image, VkImageView& imageView);

        static void createTextureSampler(VkDevice& device, VkSampler& sampler);
    };

    struct Texture{
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        uint32_t count;
        uint64_t descriptorIndex;

        Texture(std::string filepath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);
        Texture();
        void terminate(VkDevice& device);
    };
}