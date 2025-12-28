#pragma once
#include "../headers/Prometheus.h"

class WorkerThread;
struct Job;
struct PrepareForJoinJob;

class Engine {

public:
	//WINDOW
	static GLFWwindow* window;
	static const int WIDTH;
	static const int HEIGHT;

	static bool framebufferResized;

	//SYNC OBJECTS
	static std::counting_semaphore<INT_MAX> jobInQueueSem;
	static std::mutex jobQueueMutex;

	//THREADS
	static std::unordered_map<std::thread::id, WorkerThread*> threadPool;
	static std::queue<Job*> jobQueue;

	//CORE
	static void run(Engine* engine);
	static void initWindow(Engine* engine);
	static void initVulkan();
	static void mainLoop();

	//USED BY GLFW TO NOTIFY WINDOW RESIZE
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

	//THREADS
	static void initThreadPool(uint16_t poolSize);
	static void killThreadPool();
};