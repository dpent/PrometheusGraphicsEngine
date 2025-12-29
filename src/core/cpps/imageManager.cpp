#include "../headers/imageManager.h"
#include "../headers/engine.h"

void ImageManager::createImageView(VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels,
    VkImageViewType viewType, uint32_t layerCount, uint32_t baseArrayLayer, VkImageView& imageView) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = viewType;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mipLevels;
    createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
    createInfo.subresourceRange.layerCount = layerCount;

    if (vkCreateImageView(Engine::deviceInfo.logicalDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image views!");
    }
}

void ImageManager::createImage(
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
    uint32_t maxLayers
) 
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = mipLevels;
    imageInfo.arrayLayers = maxLayers;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = numSamples;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(Engine::deviceInfo.logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Engine::deviceInfo.logicalDevice, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = DeviceManager::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(Engine::deviceInfo.logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(Engine::deviceInfo.logicalDevice, image, imageMemory, 0);
}


void ImageManager::createDepthResources() {

    VkFormat format = Engine::findDepthFormat();

    ImageManager::createImage(Engine::swapChainInfo.extent.width,
        Engine::swapChainInfo.extent.height, format,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Engine::depthResource.image,
        Engine::depthResource.memory, 1, Engine::msaaSamples
    );

    ImageManager::createImageView(Engine::depthResource.image, format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, VK_IMAGE_VIEW_TYPE_2D, 1, 0, Engine::depthResource.view);
    //No need for transitioning this image since the render pass will take care of it
}


void ImageManager::createColorResources() {

    VkFormat format = Engine::swapChainInfo.imageFormat;

    ImageManager::createImage(Engine::swapChainInfo.extent.width,
        Engine::swapChainInfo.extent.height, format,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Engine::colorResource.image,
        Engine::colorResource.memory, 1, Engine::msaaSamples
    );

    ImageManager::createImageView(Engine::colorResource.image, format, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D, 1, 0, Engine::colorResource.view);
}