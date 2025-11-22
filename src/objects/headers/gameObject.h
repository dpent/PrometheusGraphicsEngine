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
#include "../../engine/headers/doubleEndedQueue.h"
#include <unordered_set>
#include "../../engine/headers/cell.h"
#include <array>

namespace Prometheus{
    
    class GameObject{
    public:
        static uint64_t autoIncrementId;
        uint64_t id;
        std::string texturePath;
        std::string meshPath;
        Mesh* mesh = nullptr;
        std::array<glm::vec3, 8> hitboxPoints;
        glm::vec3 center;
        float radius;
        Transform transform;
        InstanceInfo* info;
        std::unordered_set<Cell*> cells;
        glm::vec3 velocity;
        GameObject* next = nullptr;
        GameObject* prev = nullptr;
        bool moved = false;

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

        void drawOBB(glm::vec3 color = COLOR_MAGENTA);
        static glm::vec3 hsvToRgb(float h, float s, float v);
        static glm::vec3 getColorBasedOnTime();
        glm::vec3 updatePulse(float time);

        std::string toString();
        void animateCircularMotion(float centerX, float centerY, float centerZ, float radius, float speed, float offset);
        
        static void createObjectThreaded(std::string texturePath,std::string modelPath,VkDevice& device, VkPhysicalDevice& physicalDevice,VkQueue& graphicsQueue);
        static void deleteObjectThreaded(VkDevice &device, GameObject* object);

        bool sphereTest(glm::vec3 center,glm::vec3 radius);

        bool checkCollisions();
        std::array<glm::vec3, 8> getWorldHitpoints();

        glm::vec3 getCenter();

        void move();
    };
}