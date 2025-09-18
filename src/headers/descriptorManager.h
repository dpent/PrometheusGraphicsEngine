#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    class DescriptorManager{
    public:
        static void createDescriptorSetLayout();
    };
}