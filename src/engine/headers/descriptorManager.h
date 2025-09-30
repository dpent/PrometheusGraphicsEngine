#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    class DescriptorManager{
    public:
        static void createDescriptorSetLayout(VkDevice& device);
        static void createDescriptorPool(VkDevice& device);
        static void createDescriptorSets(VkDevice& device);
        static void recreateDescriptors(VkDevice& device);
    };
}