#include "../headers/swapChainManager.h"
#include "../headers/engine.h"
#include "../headers/queueManager.h"
#include "../headers/bufferManager.h"
#include <vulkan/vulkan_core.h>

using namespace Prometheus;

namespace Prometheus{

    SwapChainSupportDetails SwapChainSupportDetails::querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface){
        SwapChainSupportDetails details;


        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){

        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChainManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(Engine::window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void SwapChainManager::createSwapChain(VkSurfaceKHR& surface, VkPhysicalDevice& physicalDevice, VkDevice& device, VkSwapchainKHR oldSwapChain){
        SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = SwapChainManager::chooseSwapSurfaceFormat(swapChainSupport.formats);
        Engine::presentMode = SwapChainManager::chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = SwapChainManager::chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; /*Simply sticking to this minimum means that we may sometimes have to
                                                                                wait on the driver to complete internal operations before we can acquire
                                                                                another image to render to.*/
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;/*It is also possible that you'll render images to a separate
                                                                    image first to perform operations like post-processing. In that
                                                                    case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                                                    instead and use a memory operation to transfer the rendered image
                                                                    to a swap chain image.*/

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional 
            /*
            -- VK_SHARING_MODE_EXCLUSIVE: An image is owned by one queue family at a time and
            ownership must be explicitly transferred before using it in another queue family.
            This option offers the best performance.

            -- VK_SHARING_MODE_CONCURRENT: Images can be used across
            multiple queue families without explicit ownership transfers.
            */
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = Engine::presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapChain; //Necessary for smooth transitions e.g. window resize

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &Engine::swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, Engine::swapChain, &imageCount, nullptr); //We need to fill swapChainImages vector
        Engine::swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, Engine::swapChain, &imageCount, Engine::swapChainImages.data());

        Engine::swapChainImageFormat = surfaceFormat.format;
        Engine::swapChainExtent = extent;
    }

    void SwapChainManager::createImageViews(VkDevice& device){
        Engine::swapChainImageViews.resize(Engine::swapChainImages.size());

        for (size_t i = 0; i < Engine::swapChainImages.size(); i++) {
            Engine::swapChainImageViews[i]=SwapChainManager::createImageView(device,Engine::swapChainImages[i],
            Engine::swapChainImageFormat,VK_IMAGE_ASPECT_COLOR_BIT, 1);

        }
    }

    VkImageView SwapChainManager::createImageView(VkDevice& device, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels){
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectFlags;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipLevels;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }

        return imageView;
    }

    void SwapChainManager::recreateSwapChain(VkSurfaceKHR& surface,
            VkPhysicalDevice& physicalDevice, 
            VkDevice& device,
            VkQueue& presentQueue)
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(Engine::window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(Engine::window, &width, &height);
            glfwWaitEvents();
        }

        VkSwapchainKHR oldSwapChain = Engine::swapChain;
        vkQueueWaitIdle(presentQueue);

        SwapChainManager::cleanupSwapChainDependents(device);

        SwapChainManager::createSwapChain(surface,physicalDevice,device,oldSwapChain);

        SwapChainManager::createImageViews(device);
        
        BufferManager::createColorResources(device, physicalDevice);
        BufferManager::createDepthResources(device,physicalDevice);
        BufferManager::createFrameBuffers(device);

        if (oldSwapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, oldSwapChain, nullptr);
            oldSwapChain = VK_NULL_HANDLE;
        }
    }

    void SwapChainManager::cleanupSwapChain(VkDevice& device){

        for (auto framebuffer : Engine::swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        Engine::swapChainFramebuffers.clear();

        for (auto imageView : Engine::swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }
        Engine::swapChainImageViews.clear();

        vkDestroyImageView(device, Engine::depthImageView, nullptr);
        Engine::depthImageView = VK_NULL_HANDLE;
        vkDestroyImage(device, Engine::depthImage, nullptr);
        Engine::depthImage = VK_NULL_HANDLE;
        vkFreeMemory(device, Engine::depthImageMemory, nullptr);
        Engine::depthImageMemory = VK_NULL_HANDLE;

        vkDestroyImageView(device, Engine::colorImageView, nullptr);
        Engine::colorImageView=VK_NULL_HANDLE;
        vkDestroyImage(device, Engine::colorImage, nullptr);
        Engine::colorImage=VK_NULL_HANDLE;
        vkFreeMemory(device, Engine::colorImageMemory, nullptr);
        Engine::colorImageMemory=VK_NULL_HANDLE;

        if (Engine::swapChain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(device, Engine::swapChain, nullptr);
            Engine::swapChain = VK_NULL_HANDLE;
        }
    }

    void SwapChainManager::cleanupSwapChainDependents(VkDevice& device){
        for (auto framebuffer : Engine::swapChainFramebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }

        for (auto imageView : Engine::swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr);
        }

        vkDestroyImageView(device, Engine::depthImageView, nullptr);
        Engine::depthImageView = VK_NULL_HANDLE;
        vkDestroyImage(device, Engine::depthImage, nullptr);
        Engine::depthImage = VK_NULL_HANDLE;
        vkFreeMemory(device, Engine::depthImageMemory, nullptr);
        Engine::depthImageMemory = VK_NULL_HANDLE;

        vkDestroyImageView(device, Engine::colorImageView, nullptr);
        Engine::colorImageView=VK_NULL_HANDLE;
        vkDestroyImage(device, Engine::colorImage, nullptr);
        Engine::colorImage=VK_NULL_HANDLE;
        vkFreeMemory(device, Engine::colorImageMemory, nullptr);
        Engine::colorImageMemory=VK_NULL_HANDLE;
    }
    
}