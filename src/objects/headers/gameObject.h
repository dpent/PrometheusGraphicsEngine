#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../../engine/headers/textureManager.h"
#include <string>

namespace Prometheus{
    class GameObject{
    public:
        static uint64_t autoIncrementId;
        
        uint64_t textureVecIndex;
        uint64_t id;
        const char * texturePath;
        std::string meshPath;

        GameObject(const char* texturePath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue,
        std::string meshPath);
        GameObject();
        ~GameObject();
        void terminate(VkDevice& device);
        virtual void draw(VkCommandBuffer& commandBuffer);
        std::string toString();
    };
}