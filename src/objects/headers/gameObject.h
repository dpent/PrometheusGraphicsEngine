#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../../engine/headers/textureManager.h"

namespace Prometheus{
    class GameObject{
    public:
        static uint64_t autoIncrementId;

        uint64_t id;
        const char * texturePath;

        GameObject(const char* texturePath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue);
        ~GameObject();
        void terminate(VkDevice& device);
    };
}