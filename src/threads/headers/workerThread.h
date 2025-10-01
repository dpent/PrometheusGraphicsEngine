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

namespace Prometheus{

    class WorkerThread{

        std::thread thread;
        std::unordered_map<std::thread::id, WorkerThread*>* threadPool;
        std::unordered_map<std::thread::id, WorkerThread*>* activeThreads;
    
    public:
        std::queue<Job*> jobs;
        sem_t workSemaphore;
        std::thread::id id;
        std::mutex* poolMutex;

        bool alive=true;

        WorkerThread(std::mutex* poolMutex, std::unordered_map<std::thread::id, WorkerThread*>* threadPool, std::unordered_map<std::thread::id, WorkerThread*>* activeThreads);

        void workerLoop();

        void doWork(Job* job);

        void detach();
    };
}