#pragma once
#include "../../core/headers/Prometheus.h"

class Engine;
struct Job;

class WorkerThread {

public:
    std::thread::id id;
    std::thread thread;

    bool alive = true;

    WorkerThread();

    void workerLoop();

    void doWork(Job* job);

    void detach();

    void createPoolAndBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface);
};