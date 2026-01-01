#include "../headers/swapChainManager.h"
#include "../headers/engine.h"

SwapChainSupportDetails SwapChainSupportDetails::querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
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

void SwapChainManager::createSwapChain(VkSwapchainKHR oldSwapChain) {
    
    SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(Engine::deviceInfo.physicalDevice, Engine::surface);

    VkSurfaceFormatKHR surfaceFormat = SwapChainManager::chooseSwapSurfaceFormat(swapChainSupport.formats);
    Engine::swapChainInfo.presentMode = SwapChainManager::chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = SwapChainManager::chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; /*Simply sticking to this minimum means that we may sometimes have to
                                                                            wait on the driver to complete internal operations before we can acquire
                                                                            another image to render to.*/
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = Engine::surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                                                                /*It is also possible that you'll render images to a separate
                                                                image first to perform operations like post-processing. In that
                                                                case you may use a value like VK_IMAGE_USAGE_TRANSFER_DST_BIT
                                                                instead and use a memory operation to transfer the rendered image
                                                                to a swap chain image.*/

    QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(Engine::deviceInfo.physicalDevice, Engine::surface);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
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
    createInfo.presentMode = Engine::swapChainInfo.presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain; //Necessary for smooth transitions e.g. window resize

    if (vkCreateSwapchainKHR(Engine::deviceInfo.logicalDevice, &createInfo, nullptr, &Engine::swapChainInfo.chain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(Engine::deviceInfo.logicalDevice, Engine::swapChainInfo.chain, &imageCount, nullptr); //We need to fill swapChainImages vector
    Engine::swapChainInfo.images.resize(imageCount);
    vkGetSwapchainImagesKHR(Engine::deviceInfo.logicalDevice, Engine::swapChainInfo.chain, &imageCount, Engine::swapChainInfo.images.data());

    Engine::swapChainInfo.imageFormat = surfaceFormat.format;
    Engine::swapChainInfo.extent = extent;

    for (size_t i = 0; i < Engine::swapChainInfo.images.size(); i++) {
        ImageManager::createImage
        (
            Engine::swapChainInfo.extent.width,
            Engine::swapChainInfo.extent.height,
            Engine::swapChainInfo.imageFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Engine::imGuiHelperImages[i].image,
            Engine::imGuiHelperImages[i].memory,
            1,
            VK_SAMPLE_COUNT_1_BIT,
            1
        );
    }
}

VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {

    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
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

void SwapChainManager::createSwapChainImageViews() {
    Engine::swapChainInfo.imageViews.resize(Engine::swapChainInfo.images.size());
    Engine::swapChainInfo.imGuiIds.resize(Engine::swapChainInfo.images.size());

    Engine::imGuiHelperImages.resize(Engine::swapChainInfo.images.size());

    for (size_t i = 0; i < Engine::swapChainInfo.images.size(); i++) {
        ImageManager::createImageView(
            Engine::swapChainInfo.images[i],
            Engine::swapChainInfo.imageFormat, 
            VK_IMAGE_ASPECT_COLOR_BIT, 
            1, 
            VK_IMAGE_VIEW_TYPE_2D,
            1,
            0,
            Engine::swapChainInfo.imageViews[i]
        );
    }

    for (size_t i = 0; i < Engine::swapChainInfo.images.size(); i++) {
        ImageManager::createImageView(
            Engine::imGuiHelperImages[i].image,
            Engine::swapChainInfo.imageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT,
            1,
            VK_IMAGE_VIEW_TYPE_2D,
            1,
            0,
            Engine::imGuiHelperImages[i].view
        );
    }
}

void SwapChainManager::cleanupSwapChainDependents() {

    for (auto framebuffer : Engine::swapChainInfo.frameBuffers) {
        vkDestroyFramebuffer(Engine::deviceInfo.logicalDevice, framebuffer, nullptr);
    }

    for (auto imageView : Engine::swapChainInfo.imageViews) {
        vkDestroyImageView(Engine::deviceInfo.logicalDevice, imageView, nullptr);
    }

    vkDestroyImageView(Engine::deviceInfo.logicalDevice, Engine::depthResource.view, nullptr);
    Engine::depthResource.view = VK_NULL_HANDLE;
    vkDestroyImage(Engine::deviceInfo.logicalDevice, Engine::depthResource.image, nullptr);
    Engine::depthResource.image = VK_NULL_HANDLE;
    vkFreeMemory(Engine::deviceInfo.logicalDevice, Engine::depthResource.memory, nullptr);
    Engine::depthResource.memory = VK_NULL_HANDLE;

    vkDestroyImageView(Engine::deviceInfo.logicalDevice, Engine::colorResource.view, nullptr);
    Engine::colorResource.view = VK_NULL_HANDLE;
    vkDestroyImage(Engine::deviceInfo.logicalDevice, Engine::colorResource.image, nullptr);
    Engine::colorResource.image = VK_NULL_HANDLE;
    vkFreeMemory(Engine::deviceInfo.logicalDevice, Engine::colorResource.memory, nullptr);
    Engine::colorResource.memory = VK_NULL_HANDLE;
}

void SwapChainManager::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(Engine::window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Engine::window, &width, &height);
        glfwWaitEvents();
    }

    VkSwapchainKHR oldSwapChain = Engine::swapChainInfo.chain;


    vkQueueWaitIdle(Engine::queues.present);


    SwapChainManager::cleanupSwapChainDependents();

    SwapChainManager::createSwapChain(oldSwapChain);

    SwapChainManager::createSwapChainImageViews();

    ImageManager::createColorResources();
    ImageManager::createDepthResources();
    BufferManager::createFrameBuffers(
        Engine::swapChainInfo.frameBuffers, 
        Engine::swapChainInfo.imageViews, 
        Engine::swapChainInfo.extent, 
        Engine::graphicsRenderPass, 
        Engine::colorResource.view, 
        Engine::depthResource.view
    );

    if (oldSwapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(Engine::deviceInfo.logicalDevice, oldSwapChain, nullptr);
        oldSwapChain = VK_NULL_HANDLE;
    }
}

void SwapChainManager::createImGuiTexture(uint32_t imageIndex) {
    Engine::swapChainInfo.imGuiIds[imageIndex] = ImGui_ImplVulkan_AddTexture(
        Engine::linearSampler,
        Engine::swapChainInfo.imageViews[imageIndex],
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}