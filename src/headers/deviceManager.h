#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <map>
#include "../headers/queueManager.h"
#include "../headers/instanceManager.h"

namespace Prometheus{
    class DeviceManager{
    public:
        //Physical device
        static void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice physicalDevice);
        static bool rateDeviceSuitability(const VkPhysicalDevice device);

        //Logical device
        static void createLogicalDevice(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue graphicsQueue);
    };
}