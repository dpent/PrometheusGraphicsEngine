#pragma once
#include "../../core/headers/Prometheus.h"
#include "../../core/headers/bufferManager.h"

class Engine;
struct Job;

class WorkerThread {

public:
    std::thread::id id;
    std::thread thread;

    bool alive = true;

    CommandPool command;

    WorkerThread();

    void workerLoop();

    void doWork(Job* job);

    void detach();
};