#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Prometheus{
    class TextureManager{
    public:
        static void createTextureImage(const char * filename, int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice);
    };
}