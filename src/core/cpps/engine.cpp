#include "../headers/engine.h"
#include "../../threads/headers/workerThread.h"
#include "../../threads/headers/job.h"

//CORE
VulkanInstanceInfo Engine::vkInstanceInfo;
DeviceInfo Engine::deviceInfo;
SwapChainInfo Engine::swapChainInfo;

VkSampleCountFlagBits Engine::msaaSamples;

QueueHolder Engine::queues;

VkRenderPass Engine::graphicsRenderPass;
Image Engine::depthResource;
Image Engine::colorResource;

//WINDOW
GLFWwindow* Engine::window = nullptr;
const int Engine::WIDTH = 1280;
const int Engine::HEIGHT = 720;

bool Engine::framebufferResized = false;
VkSurfaceKHR Engine::surface;

//SYNC OBJECTS
std::counting_semaphore<INT_MAX> Engine::jobInQueueSem(0);
std::mutex Engine::jobQueueMutex;

//THREADS
std::unordered_map<std::thread::id, WorkerThread*> Engine::threadPool;
std::queue<Job*> Engine::jobQueue;

void Engine::run(Engine* engine) {

	Engine::initWindow(engine);
	Engine::initVulkan();
	Engine::mainLoop();
}

void Engine::initWindow(Engine* engine) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    Engine::window = glfwCreateWindow(WIDTH, HEIGHT, "Prometheus", nullptr, nullptr);
    glfwSetWindowUserPointer(Engine::window, engine);
    glfwSetFramebufferSizeCallback(Engine::window, frameBufferResizeCallback);

    /*Engine::cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    glfwSetCursor(Engine::window, Engine::cursor);

    Engine::lastKnownMousePos = std::pair<double, double>(0.0f, 0.0f);
    Engine::rightMouseFirstPress = true;
    Engine::rightMousePressedLastFrame = false;*/
}

void Engine::initVulkan() {

    Engine::initThreadPool(std::thread::hardware_concurrency() - 1);

    InstanceManager::createInstance(Engine::vkInstanceInfo.instance);
    InstanceManager::setupDebugMessenger(Engine::vkInstanceInfo.instance, Engine::vkInstanceInfo.debugMessenger);

    Engine::createSurface();

    DeviceManager::pickPhysicalDevice();
    DeviceManager::createLogicalDevice();

    SwapChainManager::createSwapChain(Engine::swapChainInfo.chain);
    SwapChainManager::createSwapChainImageViews();

    RenderPassManager::createRenderPass();

    ImageManager::createDepthResources();
    ImageManager::createColorResources();
}

void Engine::mainLoop() {
    
    while (!glfwWindowShouldClose(Engine::window)) {
        glfwPollEvents();
    }

    Engine::killThreadPool();
}

void Engine::frameBufferResizeCallback(GLFWwindow* window, int width, int height) {
    Engine::framebufferResized = true;
}

void Engine::initThreadPool(uint16_t poolSize) {

    std::cout << "\nThreads in pool: " << poolSize << "\n" << std::endl;
    for (uint16_t i = 0; i < poolSize; i++) {

        WorkerThread* wt = new WorkerThread();

        Engine::threadPool[wt->id] = wt;
    }
}

void Engine::killThreadPool() {

    Engine::jobQueueMutex.lock();

    while (!Engine::jobQueue.empty()) {
        Engine::jobQueue.pop();
    }

    for (size_t i = 0; i < Engine::threadPool.size(); i++) {
    
        PrepareForJoinJob* j = new PrepareForJoinJob();
        Engine::jobQueue.push(j);
    }

    Engine::jobQueueMutex.unlock();

    for (const auto& pair : Engine::threadPool) {
        pair.second->alive = false;
    }

    for (size_t i = 0; i < Engine::threadPool.size(); i++) {
        Engine::jobInQueueSem.release();
    }

    int j = 0;
    for (const auto& pair : Engine::threadPool) {
        if (pair.second->thread.joinable()) {
            j++;
            pair.second->thread.join();
        }
    }

    for (const auto& pair : Engine::threadPool) {
        delete pair.second;
    }

}

void Engine::createSurface() {
    if (glfwCreateWindowSurface(Engine::vkInstanceInfo.instance, Engine::window, nullptr, &Engine::surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}


VkSampleCountFlagBits Engine::getMaxUsableSampleCount() {

    VkSampleCountFlags counts = Engine::deviceInfo.physicalProperties.limits.framebufferColorSampleCounts & Engine::deviceInfo.physicalProperties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

    return VK_SAMPLE_COUNT_1_BIT;
}

VkFormat Engine::findDepthFormat() {
    return Engine::findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat Engine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

    for (VkFormat format : candidates) {

        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(Engine::deviceInfo.physicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}