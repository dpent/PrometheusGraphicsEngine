#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    class SyncManager{
    public:
        static void createSyncObjects(VkDevice& device);
    };
}