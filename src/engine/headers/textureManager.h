#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "../../engine/headers/vertex.h"

namespace Prometheus{
    class TextureManager{
    public:
        static uint32_t createTextureImage(std::string filename, int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice,
        VkImage& image, VkDeviceMemory& imageMemory, VkQueue& graphicsQueue);

        static void createImage(uint32_t width, uint32_t height, VkFormat format, 
            VkImageTiling tiling, VkImageUsageFlags usage, 
            VkMemoryPropertyFlags properties, VkImage& image, 
            VkDeviceMemory& imageMemory,
            VkDevice& device,
            VkPhysicalDevice& physicalDevice,
            uint32_t mipLevels,
            VkSampleCountFlagBits numSamples
        );
        
        static void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
        VkDevice& device, VkQueue& graphicsQueue, uint32_t mipLevels);

        static void copyBufferToImage(VkBuffer& buffer, VkImage& image, const uint32_t& width, const uint32_t& height,
        VkDevice& device, VkQueue& graphicsQueue);

        static void createTextureImageView(VkDevice& device, VkImage& image, VkImageView& imageView, uint32_t mipLevels);

        static void createTextureSampler(VkDevice& device, VkSampler& sampler);

        static void generateMipMaps(VkImage& image, int32_t& texWidth, 
            int32_t& texHeight, uint32_t& mipLevels, 
            VkDevice& device, VkQueue& graphicsQueue,
            VkFormat imageFormat, VkPhysicalDevice& physicalDevice
        );
    };

    struct Texture{
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        uint32_t count;
        uint64_t descriptorIndex;
        uint32_t mipLevels;

        Texture(std::string filepath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);
        Texture();
        void terminate(VkDevice& device);
    };
}