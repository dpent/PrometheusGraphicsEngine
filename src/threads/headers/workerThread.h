#pragma once

#include <unordered_map>
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
        std::queue<Job*> jobs;
        std::mutex jobsMutex;
        sem_t workSemaphore;
        std::thread::id id;
        std::thread thread;
        
        bool alive=true;

        WorkerThread();

        void workerLoop();

        void doWork(Job* job);

        void detach();
    };
}