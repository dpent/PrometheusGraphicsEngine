#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include "workerThread.h"
#include <unordered_map>
#include <queue>
#include <random>
#include <mutex>
#include <semaphore.h>

namespace Prometheus{
    class ThreadManager{
        std::thread managerThread;

        std::unordered_map<std::thread::id, WorkerThread*> threadPool;
    
    public:
        std::queue<Job> jobQueue;
        std::mutex queueMutex;
        sem_t workInQueueSemaphore;
        bool alive=true;

        ThreadManager();

        void managerLoop();

        void start(uint16_t poolSize);

        void detach();

        std::vector<std::queue<Job*>> batchJobs();

        void terminate();
    };
}