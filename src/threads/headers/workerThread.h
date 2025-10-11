#pragma once

#include <unordered_map>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <queue>
#include <thread>
#include "job.h"
#include <semaphore.h>
#include <iostream>
#include <chrono>
#include <mutex>
#include <string>

namespace Prometheus{

    class WorkerThread{
    
    public:
        std::queue<Job> jobs;
        std::mutex jobsMutex;
        std::thread::id id;
        std::thread thread;

        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;
        
        bool alive=true;

        WorkerThread(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);

        void workerLoop();

        void doWork(Job* job);

        void detach();

        void createPoolAndBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
    };
}