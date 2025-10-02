#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <map>
#include "../headers/queueManager.h"
#include "../headers/instanceManager.h"
#include <set>

namespace Prometheus{
    class DeviceManager{
    public:
        //Physical device
        static void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);
        static bool rateDeviceSuitability(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
        static bool checkDeviceExtensionSupport(const VkPhysicalDevice& device);

        //Logical device
        static void createLogicalDevice(const VkPhysicalDevice& physicalDevice,
            VkDevice& device,
            VkQueue& graphicsQueue,
            VkQueue& presentQueue,
            VkSurfaceKHR& surface
        );

        static const char* deviceTypeToString(VkPhysicalDeviceType& deviceType);
        static const char* vendorIdToString(uint32_t& vendorId);
    };
}