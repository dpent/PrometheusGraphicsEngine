#pragma once

#include "Prometheus.h"

class Engine;

struct Image {
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;

    void destroy();
};

struct ImageVector {
    std::vector<VkImage> images;
    std::vector<VkDeviceMemory> memories;
    std::vector<VkImageView> views;

    void destroyAllItems();
    void resize(uint32_t size);
};

class ImageManager {
public:
    static void createImageView(
        VkImage& image,
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t mipLevels,
        VkImageViewType viewType,
        uint32_t layerCount,
        uint32_t baseArrayLayer,
        VkImageView& imageView
    );

    static void createImage(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory,
        uint32_t mipLevels,
        VkSampleCountFlagBits numSamples,
        uint32_t maxLayers = 1
    );

    static void createImageSampler(VkSampler& sampler);


    static void createDepthResources();
    static void createColorResources();
    static void createShadowMapResources();

    static void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
        uint32_t mipLevels, VkCommandPool& commandPool);

    static void transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
        uint32_t mipLevels, VkCommandBuffer& commandBuffer);

    static void copyBufferToImage(VkBuffer& buffer, VkImage& image, const uint32_t& width, const uint32_t& height,
        VkCommandPool& commandPool);

    static void generateMipMaps(VkImage& image, int32_t& texWidth,
        int32_t& texHeight, uint32_t& mipLevels,
        VkFormat imageFormat, VkCommandPool& commandPool
    );

    static void createDummyImage();

    static void createSolidColorFilePNG(std::string filename, unsigned char r, unsigned char g, unsigned char b);

    static void createAccumulationImage(Image& image);
};