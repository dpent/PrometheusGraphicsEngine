#include "../headers/queueInfo.h"
#include "../headers/engine.h"

QueueFamilyIndices QueueFamilyIndices::findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
            Engine::queues.presentIndex = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
            (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
            indices.graphicsFamily = i;
            Engine::queues.graphicsIndex = i;
            indices.computeFamily = i;
            Engine::queues.computeIndex = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }

    return indices;
}