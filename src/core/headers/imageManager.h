#pragma once

#include "Prometheus.h"

class Engine;

class ImageManager {
public:
	static void createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels,
		VkImageViewType viewType, uint32_t layerCount, uint32_t baseArrayLayer, VkImageView& imageView);
};