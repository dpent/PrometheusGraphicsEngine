#pragma once
#include "Prometheus.h"
#include "instanceManager.h"
#include "deviceManager.h"
#include "renderPassManager.h"
#include "bufferManager.h"
#include "pipelineManager.h"
#include "descriptorManager.h"
#include "../../gameObjects/headers/gameObject.h"
#include "../../imgui/guiManager.h"
#include "../../gameObjects/headers/camera.h"
#include "inputManager.h"
#include "../../gameObjects/headers/material.h"

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

	static std::filesystem::path exeDir;

	static const uint32_t MAX_TEXTURES = 1024;

	inline static const ImVec4 IMGUI_BACKGROUND_COLOR = ImVec4(0.2f, 0.0f, 0.0f, 3.0f);
	inline static const ImVec4 IMGUI_ACTIVE_COLOR = ImVec4(0.6f, 0.0f, 0.0f, 1.0f);
	inline static const ImVec4 IMGUI_HIGHLIGHT_COLOR = ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
	inline static const ImVec4 IMGUI_DARK_COLOR = ImVec4(0.05f, 0.0f, 0.0f, 1.0f);

	inline static const glm::vec3 WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

	//CORE
	static VulkanInstance vkInstanceInfo;
	static DeviceInfo deviceInfo;
	static SwapChain swapChainInfo;
	static ImageVector imGuiHelperImages;

	static VkSampleCountFlagBits msaaSamples;

	static QueueHolder queues;

	static VkRenderPass graphicsRenderPass;
	static Image depthResource;
	static Image colorResource;

	static Pipeline graphicsPipeLine;
	static Descriptor graphicsDescriptor;

	static Descriptor imGuiDescriptor;

	static std::deque<Material*> materials;
	static std::deque<Texture*> textures;
	static std::deque<GameObject*> gameObjects;
	static std::deque<Mesh*> meshes;
	static std::vector<InstanceInfo> instanceData;

	static CommandPool command;

	static VertexData vertexIndexData;
	static Buffer vertexIndexBuffer;

	static Buffer stagingBuffer;
	static Buffer instanceDataSSBO;

	static uint8_t currentFrame;

	static std::vector<VkFence> inFlightFences;
	static std::vector<VkSemaphore> imageAvailableSemaphores;
	static std::vector<VkSemaphore> renderFinishedSemaphores;

	static VkSampler linearSampler;

	static std::vector<bool> pressed;

	static glm::vec3 worldUp;

	static bool firstFrame;
	static bool remakeDescriptors;
	static bool remakeVertexIndexBuffer;
	static bool remakeInstanceDataSSBO;

	//WINDOW
	static GLFWwindow* window;
	static GLFWcursor* cursor;
	static const int WIDTH;
	static const int HEIGHT;

	static std::pair<double, double> lastKnownMousePos;
	static bool rightMouseFirstPress;
	static bool rightMousePressedLastFrame;

	static bool framebufferResized;
	static VkSurfaceKHR surface;

	static Camera camera;

	static bool displayGUI;

	//SYNC OBJECTS
	static std::counting_semaphore<INT_MAX> jobInQueueSem;
	static std::mutex jobQueueMutex;

	static std::mutex objectCreateMutex;
	static std::mutex materialMutex;
	static std::mutex meshMutex;
	static std::mutex textureMutex;

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
	
	static std::vector<char> readFile(const std::string& filename);
	static VkShaderModule createShaderModule(const std::vector<char>& code);
	static VkPipelineShaderStageCreateInfo createShaderStageInfo(VkStructureType sType,
		VkShaderStageFlagBits stage,
		VkShaderModule& module,
		const char* pName,
		const VkSpecializationInfo* pSpecializationInfo = nullptr);

	static void drawFrame();
	static void prepareFrameData();

	static void createSyncObjects();

	static void recreateVertexIndexData();
	static void updateObjects();

	//USED BY GLFW TO NOTIFY WINDOW RESIZE
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);

	//THREADS
	static void initThreadPool(uint16_t poolSize);
	static void killThreadPool();

	//FORMATS
	static VkFormat findDepthFormat();
	static VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};