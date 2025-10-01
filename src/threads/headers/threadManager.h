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
        std::mutex queueMutex;

        std::mutex poolMutex;

        std::unordered_map<std::thread::id, WorkerThread*> threadPool;
        std::unordered_map<std::thread::id, WorkerThread*> activeThreads;   
    
    public:
        std::queue<Job> jobQueue;
        bool alive=true;

        ThreadManager();

        void managerLoop();

        void start(uint16_t poolSize);

        void detach();
    };
}