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
        static void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
        static bool rateDeviceSuitability(const VkPhysicalDevice& device, VkSurfaceKHR& surface);

        //Logical device
        static void createLogicalDevice(const VkPhysicalDevice& physicalDevice,
            VkDevice& device,
            VkQueue& graphicsQueue,
            VkQueue& presentQueue,
            VkSurfaceKHR& surface
        );
    };
}