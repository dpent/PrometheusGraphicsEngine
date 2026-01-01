#pragma once

#include "Prometheus.h"

class Engine;

class ImageManager {
public:
	static void createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels,
		VkImageViewType viewType, uint32_t layerCount, uint32_t baseArrayLayer, VkImageView& imageView);

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
};

struct Image {
	VkImage image;
	VkDeviceMemory memory;
	VkImageView view;
};