#pragma once

#include "Prometheus.h"
#include "imageManager.h"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
};

struct SwapChain {
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkExtent2D extent;
    VkFormat imageFormat;
    VkSwapchainKHR chain;
    VkPresentModeKHR presentMode;
};

class SwapChainManager {
public:

    static void createSwapChain(VkSwapchainKHR oldSwapChain);
    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    static void createSwapChainImageViews();

    static void recreateSwapChain();
    static void cleanupSwapChainDependents();
};