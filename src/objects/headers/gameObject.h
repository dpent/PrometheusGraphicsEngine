#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  

namespace Prometheus{
    class GameObject{
    public:
        static uint64_t autoIncrementId;
        
        uint64_t textureVecIndex;
        uint64_t id;
        const char * texturePath;
        std::string meshPath;
        glm::mat4 modelMatrix;

        GameObject(const char* texturePath,int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue,
        std::string meshPath);
        GameObject();
        virtual ~GameObject();
        void terminate(VkDevice& device);
        virtual void draw(VkCommandBuffer& commandBuffer, uint32_t instanceCount, uint32_t firstInstance);
        std::string toString();
        glm::mat4 animateCircularMotion(float centerX, float centerY, float centerZ, float radius, float speed, float offset);
    };
}