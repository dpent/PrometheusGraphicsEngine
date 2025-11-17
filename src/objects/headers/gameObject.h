#pragma once

#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include "transform.h"

namespace Prometheus{
    
    struct InstanceInfo{
        glm::mat4 modelMatrix;
        alignas(16) uint32_t textureIndex;
    
        InstanceInfo(glm::mat4 model, uint32_t textureIndex);
        InstanceInfo();
    
        std::string toString();
    };

    class GameObject{
    public:
        static uint64_t autoIncrementId;
        
        uint64_t textureVecIndex;
        uint64_t id;
        std::string texturePath;
        std::string meshPath;
        Transform transform;
        InstanceInfo* info;

        GameObject(std::string texturePath,std::string modelPath,int req_comp, 
            VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue,
            VkCommandPool& commandPool);
        GameObject();
        virtual ~GameObject();
        void terminate(VkDevice& device);
        virtual void draw(VkCommandBuffer& commandBuffer, uint32_t instanceCount, uint32_t firstInstance);

        virtual void start();
        virtual void update();
        virtual void updateInstanceInfo(uint64_t textureIndex);

        std::string toString();
        void animateCircularMotion(float centerX, float centerY, float centerZ, float radius, float speed, float offset);
        
        static void createObjectThreaded(std::string texturePath,std::string modelPath,VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue);
        static void deleteObjectThreaded(VkDevice &device, uint64_t id);
    };


}