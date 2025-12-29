#pragma once
#include "Prometheus.h"
#include "instanceManager.h"
#include "deviceManager.h"
#include "renderPassManager.h"
#include "bufferManager.h"

class WorkerThread;
struct Job;
struct PrepareForJoinJob;

class Engine {

public:
	//IMPORTANT CONSTANTS
	inline static const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	static const int MAX_FRAMES_IN_FLIGHT = 2;

	//CORE
	static VulkanInstanceInfo vkInstanceInfo;
	static DeviceInfo deviceInfo;
	static SwapChainInfo swapChainInfo;

	static VkSampleCountFlagBits msaaSamples;

	static QueueHolder queues;

	static VkRenderPass graphicsRenderPass;
	static Image depthResource;
	static Image colorResource;

	//WINDOW
	static GLFWwindow* window;
	static const int WIDTH;
	static const int HEIGHT;

	static bool framebufferResized;
	static VkSurfaceKHR surface;

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
	
	static void createSurface();
	static VkSampleCountFlagBits getMaxUsableSampleCount();

	//USED BY GLFW TO NOTIFY WINDOW RESIZE
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

	//THREADS
	static void initThreadPool(uint16_t poolSize);
	static void killThreadPool();

	//FORMATS
	static VkFormat findDepthFormat();
	static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};