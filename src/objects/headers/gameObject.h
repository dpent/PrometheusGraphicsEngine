#pragma once

#include <cstdint>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include "transform.h"
#include "mesh.h"
#include "instanceInfo.h"
#include "../../debug/headers/debug.h"

namespace Prometheus{
    
    

    class GameObject{
    public:
        static uint64_t autoIncrementId;
        uint64_t id;
        std::string texturePath;
        std::string meshPath;
        Mesh* mesh;
        glm::vec3 hitboxPoints[8];
        Transform transform;
        InstanceInfo* info;
        GameObject* next = nullptr;
        GameObject* prev = nullptr;

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
        virtual void scale(glm::vec3 scale);
        virtual void rotate(glm::quat rotation);

        void drawAABB(glm::vec3 color = COLOR_MAGENTA);
        static glm::vec3 hsvToRgb(float h, float s, float v);
        static glm::vec3 getColorBasedOnTime();
        glm::vec3 updatePulse(float time);

        std::string toString();
        void animateCircularMotion(float centerX, float centerY, float centerZ, float radius, float speed, float offset);
        
        static void createObjectThreaded(std::string texturePath,std::string modelPath,VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue);
        static void deleteObjectThreaded(VkDevice &device, GameObject* object);
    };


}