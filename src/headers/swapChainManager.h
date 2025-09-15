#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp


namespace Prometheus{
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;

        static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    };

    class SwapChainManager{
    public:
        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        static void createSwapChain(VkSurfaceKHR& surface,
            VkPhysicalDevice& physicalDevice, 
            VkDevice& device,
            VkSwapchainKHR oldSwapChain
        );
        static void createImageViews(VkDevice& device);
        static void recreateSwapChain(VkSurfaceKHR& surface,
            VkPhysicalDevice& physicalDevice, 
            VkDevice& device,
            VkQueue& presentQueue
        );
        static void cleanupSwapChain(VkDevice& device);
        static void cleanupSwapChainDependents(VkDevice& device);
    };
    
}