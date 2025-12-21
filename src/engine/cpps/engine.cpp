#include "../headers/engine.h"
#include "../headers/deviceManager.h"
#include "../headers/swapChainManager.h"
#include "../headers/graphicsPipelineManager.h"
#include "../headers/renderPassManager.h"
#include "../headers/bufferManager.h"
#include "../headers/syncManager.h"
#include <semaphore>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "../headers/descriptorManager.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <string.h>
#include <chrono>
#include "../../threads/headers/descriptorOperations.h"
#include "../headers/cell.h"
#include "../../physics/headers/collision.h"
#include "../headers/computePipelineManager.h"

using namespace Prometheus;

//DEFINE STATIC VARIABLES BEFORE VULKAN INIT
GLFWwindow* Engine::window = nullptr;
GLFWcursor* Engine::cursor = nullptr;
VkPresentModeKHR Engine::presentMode=VK_PRESENT_MODE_MAILBOX_KHR;

std::vector<VkImage> Engine::swapChainImages;
std::vector<VkImageView> Engine::swapChainImageViews;
VkExtent2D Engine::swapChainExtent;
VkFormat Engine::swapChainImageFormat;
VkSwapchainKHR Engine::swapChain;

VkDescriptorSetLayout Engine::descriptorSetLayout;
VkDescriptorSetLayout Engine::computeSetLayout;
VkPipelineLayout Engine::pipelineLayout;
VkRenderPass Engine::renderPass;
VkPipeline Engine::graphicsPipeline;
VkPipeline Engine::preGraphicsPipeline;
VkPipelineLayout Engine::preGraphicsLayout;
VkPipeline Engine::debugPipeline;
VkPipelineLayout Engine::debugPipelineLayout;
VkPipeline Engine::lightBillboardPipeline;
VkPipelineLayout Engine::lightPipelineLayout;
VkPipeline Engine::computePipeline;
VkPipelineLayout Engine::computePipelineLayout;
VkPipeline Engine::particleGraphicsPipeline;
VkPipelineLayout Engine::particlePipelineLayout;

VkCommandPool Engine::commandPool;
std::vector<VkCommandBuffer> Engine::commandBuffers;
VkCommandPool Engine::computeCommandPool;
std::vector<VkCommandBuffer> Engine::computeCommandBuffers;

std::vector<VkBuffer> Engine::shaderStorageBuffers;
std::vector<VkDeviceMemory> Engine::shaderStorageBuffersMemories;
VkDeviceSize Engine::shaderStorageBufferSize;

std::vector<VkFramebuffer> Engine::swapChainFramebuffers;

std::vector<VkSemaphore> Engine::imageAvailableSemaphores;
std::vector<VkSemaphore> Engine::renderFinishedSemaphores;
std::vector<VkSemaphore> Engine::computeFinishedSemaphores;
std::vector<VkFence> Engine::computeInFlightFences;
std::vector<VkFence> Engine::inFlightFences;
std::binary_semaphore Engine::descriptorsReadySemaphore{ 0 };
std::binary_semaphore Engine::safeToMakeInstanceBuffer{ 0 };
std::binary_semaphore Engine::verIndBufferComplete{ 0 };
std::binary_semaphore Engine::instanceBufferReady{ 0 };
std::binary_semaphore Engine::commandBufferRecorded{ 0 };
std::binary_semaphore Engine::debugBuffersReady{ 0 };
std::binary_semaphore Engine::setReady{ 0 };
std::mutex Engine::gameObjectMutex;
std::mutex Engine::canDeleteObjectMutex;
std::mutex Engine::textureMutex;
std::mutex Engine::textureQueuedMutex;
std::mutex Engine::graphicsQueueMutex;
std::mutex Engine::commandPoolMutex;
std::mutex Engine::meshMutex;
std::mutex Engine::descriptorQueuedMutex;
std::mutex Engine::debugMutex;

uint32_t Engine::currentFrame = 0;

bool Engine::framebufferResized = false;

std::vector<Vertex> Engine::vertices;
std::vector<uint32_t> Engine::indices;

std::vector<Vertex> Engine::debugVertices;
std::vector<uint32_t> Engine::debugIndices;
std::unordered_map<Vertex, uint32_t> Engine::debugVertSet;

VkBuffer Engine::indexVertexBuffer= nullptr;
VkDeviceMemory Engine::indexVertexBufferMemory= nullptr;
uint64_t Engine::indexVertexBufferSize = 0;

VkBuffer Engine::debugIndexVertexBuffer = nullptr;
VkDeviceMemory Engine::debugIndexVertexBufferMemory = nullptr;
uint64_t Engine::debugIndexVertexBufferSize = 0;

std::vector<VkBuffer> Engine::instanceBuffers;
std::vector<VkDeviceMemory> Engine::instanceBufferMemories;
std::vector<void*> Engine::instanceBuffersMapped;
uint64_t Engine::instanceBufferSize = 0;

VkDeviceSize Engine::indexOffset=0;
VkDeviceSize Engine::debugIndexOffset=0;

std::vector<VkBuffer> Engine::uniformBuffers;
std::vector<VkDeviceMemory> Engine::uniformBuffersMemory;
std::vector<void*> Engine::uniformBuffersMapped;

VkDescriptorPool Engine::descriptorPool;
VkDescriptorPool Engine::imGUIPool;
std::vector<std::vector<VkDescriptorSet>> Engine::descriptorSets;
std::list<VkDescriptorPool> Engine::descriptorDeleteQueue;
std::list<int> Engine::framesSinceDescriptorQueuedForDeletion;
VkDescriptorPool Engine::computePool;
std::vector<VkDescriptorSet> Engine::computeSets;

//std::unordered_map<uint64_t,GameObject*> Engine::gameObjectMap;
DoubleEndedQueue<GameObject*> Engine::objectDQueue;

VkPhysicalDeviceProperties Engine::physicalDeviceProperties;
VkPhysicalDeviceFeatures Engine::physicalDeviceFeatures;

std::unordered_map<std::string, Texture> Engine::textureMap;
std::unordered_map<std::string, std::vector<Texture>> Engine::texturesQueuedForDeletion;
std::unordered_map<std::string, std::vector<int>> Engine::framesSinceTextureQueuedForDeletion;

std::unordered_map<std::string,Mesh> Engine::meshMap;
std::unordered_set<std::string> Engine::meshesLoading;
std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>> Engine::objectsByMesh;
std::vector<MeshBatch*> Engine::meshBatches;

VkImage Engine::depthImage;
VkDeviceMemory Engine::depthImageMemory;
VkImageView Engine::depthImageView;

bool Engine::recreateVertexIndexBuffer=true;
bool Engine::recreateInstanceBuffer=true;
bool Engine::recreateDescriptors=true;

VkSampleCountFlagBits Engine::msaaSamples=VK_SAMPLE_COUNT_1_BIT;
VkImage Engine::colorImage;
VkDeviceMemory Engine::colorImageMemory;
VkImageView Engine::colorImageView;

std::unordered_map<std::thread::id, WorkerThread*> Engine::threadPool;
std::queue<Job*> Engine::jobQueue;
std::queue<Job*> Engine::deferredJobQueue;
std::mutex Engine::queueMutex;
std::counting_semaphore<INT_MAX> Engine::workInQueueSemaphore(0);

uint64_t Engine::frameCount=0;

SafeUint16_t Engine::threadsAvailable = SafeUint16_t(std::thread::hardware_concurrency()-1);

bool Engine::wasPlacedInThread=false;

#ifdef __linux__
    std::filesystem::path Engine::exeDir = std::filesystem::canonical("/proc/self/exe").parent_path();
#elif _WIN32
    std::filesystem::path Engine::exeDir = std::filesystem::current_path();
#endif

Camera Engine::camera = Camera();

std::pair<double,double> Engine::lastKnownMousePos;
bool Engine::rightMouseFirstPress;
bool Engine::rightMousePressedLastFrame;

bool Engine::updateCameraVectors;
double Engine::updateTime = 1.0/60.0;
std::chrono::system_clock::time_point Engine::lastFrameTime = std::chrono::system_clock::now();

std::vector<bool> Engine::pressed;
glm::vec3 Engine::cameraChangePos = glm::vec3(0.0f);

uint32_t Engine::graphicsFamilyIndex;

std::chrono::system_clock::time_point Engine::lastUpdateTime = std::chrono::system_clock::now();

bool Engine::capFPS = true;
int Engine::fpsCap = 60;
double Engine::frameTime = 0.0;

std::array<std::array<std::array<Cell*, 50>, 50>, 50> Engine::spatialHash = {nullptr};
glm::vec3 Engine::cellSize;

std::unordered_map<std::string, MeshBatch*> Engine::meshSet;
std::unordered_map<std::string,uint64_t> Engine::textureIndices;

DoubleEndedQueue<UBOContainer*> Engine::lights;
bool Engine::recreateUBO = false;

std::vector<Particle> Engine::particles;
bool Engine::particlesChanged = false;

namespace Prometheus{
    void Engine::run(int argc, char** argv) {

        #ifdef __linux__
            createLinuxDesktopEntry(argv[0]);
        #endif

        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    void Engine::initWindow(){
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        Engine::window = glfwCreateWindow(WIDTH, HEIGHT, "Prometheus", nullptr, nullptr);
        glfwSetWindowUserPointer(Engine::window, this);
        glfwSetFramebufferSizeCallback(Engine::window, frameBufferResizeCallback);

        Engine::cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        glfwSetCursor(Engine::window, Engine::cursor);

        Engine::lastKnownMousePos = std::pair<double,double>(0.0f,0.0f);
        Engine::rightMouseFirstPress = true;
        Engine::rightMousePressedLastFrame = false;
    }

    void Engine::initVulkan() {
        
        Engine::pressed.resize(349, false);
        Engine::camera.updateCameraVectors();
        
        InstanceManager::createInstance(this->instance);
        InstanceManager::setupDebugMessenger(this->instance,this->debugMessenger);
        
        Engine::createSurface();
        
        DeviceManager::pickPhysicalDevice(this->instance,this->physicalDevice, this->surface);
        DeviceManager::createLogicalDevice(this->physicalDevice, this->device, this->graphicsQueue,this->presentQueue, this->surface, this->computeQueue);
        
        vkGetPhysicalDeviceProperties(physicalDevice, &Engine::physicalDeviceProperties); //We will use them for anisotropic filtering etc later on
        vkGetPhysicalDeviceFeatures(physicalDevice, &Engine::physicalDeviceFeatures);
        
        Engine::initThreadPool(std::thread::hardware_concurrency()-1,this->device, this->physicalDevice,
        this->surface);
        
        SyncManager::createSyncObjects(this->device);

        SwapChainManager::createSwapChain(this->surface,this->physicalDevice,this->device, Engine::swapChain);
        SwapChainManager::createImageViews(this->device);

        RenderPassManager::createRenderPass(this->device, this->physicalDevice);

        //Particle::addDemoParticles(4096);

        Engine::shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        Engine::shaderStorageBuffersMemories.resize(MAX_FRAMES_IN_FLIGHT);

        DescriptorManager::createComputeDescriptorSetLayout(this->device);

        DescriptorManager::createDescriptorSetLayout(this->device);

        GraphicsPipelineManager::createGraphicsPipeline(this->device);
        GraphicsPipelineManager::createLightBillBoardPipeline(this->device);
        ComputePipelineManager::createComputePipeline(this->device);
        GraphicsPipelineManager::createParticleGraphicsPipeline(this->device);

        #ifdef EDITOR
            GraphicsPipelineManager::createEditorPreGraphicsPipeline(this->device);
            GraphicsPipelineManager::createDebugPipeline(this->device);
        #endif

        BufferManager::createColorResources(this->device,this->physicalDevice);
        BufferManager::createDepthResources(this->device,this->physicalDevice);
        BufferManager::createFrameBuffers(this->device);
        BufferManager::createCommandPool(this->physicalDevice, this->surface,
            this->device,Engine::commandPool);
        BufferManager::createCommandBuffers(this->device, 
            Engine::commandBuffers, Engine::commandPool);
        BufferManager::createCommandPool(this->physicalDevice, this->surface, 
            this->device, Engine::computeCommandPool);
        BufferManager::createCommandBuffers(this->device, 
            Engine::computeCommandBuffers, Engine::computeCommandPool);

        /*BufferManager::createSSBOs(this->device, this->physicalDevice, Engine::shaderStorageBuffers,
            Engine::shaderStorageBufferSize, Engine::shaderStorageBuffersMemories, Engine::particles, this->graphicsQueue);

        DescriptorManager::createComputeDescriptorPool(this->device);
        DescriptorManager::createComputeDescriptorSets(this->device);*/

        createSpatialHash(glm::vec3(200.0f), glm::vec3(-200.0f));

        TextureManager::createSolidColorTextureFile("./textures/blue.png",0,0,255);
        TextureManager::createSolidColorTextureFile("./textures/red.png",255,0,0);
        TextureManager::createSolidColorTextureFile("./textures/magenta.png",0,255,0);
        TextureManager::createSolidColorTextureFile("./textures/magenta.png",255,0,255);
        TextureManager::createSolidColorTextureFile("./textures/white.png",255,255,255);
        
        /*for(int i=0; i<1; i++){ //100 is the safe limit

            GameObject::createObjectThreaded("../textures/magenta.png", 
                "../models/stanford_dragon.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );
    
            GameObject::createObjectThreaded("../textures/magenta.png", 
                "../models/stanford_dragon.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );
    
            GameObject::createObjectThreaded("../textures/babyBlue.png", 
                "../models/stanford_dragon.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );
    
            GameObject::createObjectThreaded("../textures/white.png", 
                "../models/stanford_dragon.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );
        }*/

		GameObject::createObjectThreaded(device, physicalDevice, graphicsQueue, new GameObject("./textures/red.png", "./models/stanford_dragon.obj"));

        Engine::instanceBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBufferMemories.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBuffersMapped.resize(Engine::MAX_FRAMES_IN_FLIGHT);

		Engine::shaderStorageBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
		Engine::shaderStorageBuffersMemories.resize(Engine::MAX_FRAMES_IN_FLIGHT);

    }

    void Engine::mainLoop() {

        createLightSource(glm::vec4(0.0f), glm::vec4(COLOR_RED,1.0f), 90.0f);
        createLightSource(glm::vec4(0.0f), glm::vec4(COLOR_BLUE,1.0f), 90.0f);

        BufferManager::createUniformBuffers(this->device,this->physicalDevice);

        InputManager::initInputMode(false,false,false,false,false,Engine::window);

        glfwSetKeyCallback(Engine::window, InputManager::keyCallBack);
        glfwSetCursorEnterCallback(Engine::window, InputManager::mouseEnterCallBack);
        glfwSetMouseButtonCallback(Engine::window, InputManager::mouseButtonCallBack);
        glfwSetScrollCallback(Engine::window, InputManager::mouseScrollCallBack);
        glfwSetCursorPosCallback(Engine::window, InputManager::cursorPosCallBack);

        Engine::commandPoolMutex.lock();
        WindowManager::initGUI(instance,graphicsQueue,device,physicalDevice);
        Engine::commandPoolMutex.unlock();

        //auto frameZeroTime = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(Engine::window)) {
            glfwPollEvents();
            InputManager::consumeInput(Engine::window);

            if(Engine::updateCameraVectors){
                Engine::camera.updateCameraVectors();
                Engine::updateCameraVectors = false;
            }

            WindowManager::startNewFrame();

            drawSpatialHash();

            drawFrame();

            //objectLoadingTest(frameZeroTime);
            
            createUpdateTextureQueueJob();
            createUpdateDescriptorQueueJob();
            
            #ifdef EDITOR
                Engine::debugVertices.clear();
                Engine::debugIndices.clear();
                Engine::debugVertSet.clear();
            #endif
        }

        Engine::graphicsQueueMutex.lock();

        vkDeviceWaitIdle(device);

        Engine::graphicsQueueMutex.unlock();

    }

    void Engine::createSurface(){
        if (glfwCreateWindowSurface(instance, Engine::window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Engine::cleanup() {

        killThreads();

        Engine::meshBatches.clear();

        if(Engine::objectDQueue.size!=0){
            
            GameObject* object = Engine::objectDQueue.head;

            while(true){

                GameObject* temp = object->next;
                
                object->terminate(device);
                delete object->info;
                delete object;
                
                if(temp == nullptr){
                    break;
                }
                
                object = temp;
            }
        }

        for (auto& [path, texVec] : Engine::texturesQueuedForDeletion) {
            for (int i = static_cast<int>(texVec.size()) - 1; i >= 0; --i) {

                auto& tex = texVec[i];
                tex.terminate(device);
            }
        }

        for(size_t i=0; i<Engine::meshBatches.size(); i++){
            delete Engine::meshBatches[i];
        }

        if(Engine::lights.size !=0){
            UBOContainer* ubo = Engine::lights.head;
            
            for(size_t i=0; i<Engine::lights.size; i++){
                delete ubo->ubo;
                UBOContainer* temp = ubo->next;
                delete ubo;

                ubo = temp;
            }
        }
        
        WindowManager::cleanup();

        vkDestroyDescriptorPool(device, Engine::imGUIPool, nullptr);

        vkDestroyDescriptorSetLayout(device,Engine::computeSetLayout, nullptr);
        vkDestroyDescriptorPool(device, Engine::computePool, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, Engine::shaderStorageBuffers[i], nullptr);
            vkFreeMemory(device, Engine::shaderStorageBuffersMemories[i], nullptr);
        }

        SwapChainManager::cleanupSwapChain(device);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(device, Engine::uniformBuffers[i], nullptr);
            vkFreeMemory(device, Engine::uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorSetLayout(device, Engine::descriptorSetLayout, nullptr);

        vkDestroyDescriptorPool(device, Engine::descriptorPool, nullptr);

        vkDestroyBuffer(device, Engine::indexVertexBuffer, nullptr);
        vkFreeMemory(device, Engine::indexVertexBufferMemory, nullptr);
        
        vkDestroyBuffer(device, Engine::debugIndexVertexBuffer, nullptr);
        vkFreeMemory(device, Engine::debugIndexVertexBufferMemory, nullptr);

        for(size_t i=0; i<MAX_FRAMES_IN_FLIGHT; i++){
            vkDestroyBuffer(device, Engine::instanceBuffers[i], nullptr);
            vkFreeMemory(device, Engine::instanceBufferMemories[i], nullptr);
        }

        for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(device, Engine::renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(device, Engine::imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(device, Engine::inFlightFences[i], nullptr);
            vkDestroySemaphore(device, Engine::computeFinishedSemaphores[i], nullptr);
            vkDestroyFence(device, Engine::computeInFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(device, commandPool, nullptr);
        vkDestroyCommandPool(device, computeCommandPool, nullptr);

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipeline(device, lightBillboardPipeline, nullptr);
        vkDestroyPipelineLayout(device, lightPipelineLayout, nullptr);
        vkDestroyPipeline(device, preGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, preGraphicsLayout, nullptr);
        vkDestroyPipeline(device, debugPipeline, nullptr);
        vkDestroyPipelineLayout(device, debugPipelineLayout, nullptr);
        vkDestroyPipeline(device, computePipeline, nullptr);
        vkDestroyPipelineLayout(device, computePipelineLayout, nullptr);
        vkDestroyPipeline(device, particleGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, particlePipelineLayout, nullptr);

        vkDestroyRenderPass(device, renderPass, nullptr);

        vkDestroyDevice(device, nullptr);

        if (InstanceManager::enableValidationLayers) {
            InstanceManager::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(Engine::window);
        glfwDestroyCursor(Engine::cursor);
        glfwTerminate();
    }

    std::vector<char> Engine::readFile(const std::string& filename) {
        std::ifstream file((Engine::exeDir / filename).lexically_normal().string(), 
        std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void Engine::drawFrame(){

        vkWaitForFences(device, 1, &Engine::inFlightFences[Engine::currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, Engine::swapChain, UINT64_MAX,
            Engine::imageAvailableSemaphores[Engine::currentFrame], 
            VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            SwapChainManager::recreateSwapChain(surface,physicalDevice,device,presentQueue);
            framebufferResized = false;
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { //Lacks logic for suboptimal swap chains
            throw std::runtime_error("failed to acquire swap chain image!");
        }


        vkResetFences(device, 1, &Engine::inFlightFences[Engine::currentFrame]);

        double timeBetweenDraws = 1.0f/Engine::fpsCap;

        std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
        std::chrono::duration<double> delta = currentTime - Engine::lastFrameTime;

        auto sleepTime = timeBetweenDraws - delta.count();

        Engine::frameTime = delta.count();

        if (sleepTime > 0.0 && Engine::capFPS) {
            std::this_thread::sleep_for(std::chrono::duration<double>(timeBetweenDraws) - delta);
        }

        if(Engine::lights.size != 0){

            Engine::updateLightSources();

            BufferManager::updateUniformBuffer(Engine::currentFrame);
        }

        if (Engine::particlesChanged) {

            updateParticleSSBOs();
            remakeComputeDescriptorSetsAndPool();

			Engine::particlesChanged = false;
        }

        VkSubmitInfo submitInfo{};

        if (Engine::particles.size() != 0) {

            BufferManager::recordComputeCommandBuffer(Engine::computeCommandBuffers[Engine::currentFrame], imageIndex,
                device, physicalDevice);

            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &Engine::computeCommandBuffers[Engine::currentFrame];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &Engine::computeFinishedSemaphores[Engine::currentFrame];

            if (vkQueueSubmit(computeQueue, 1, &submitInfo, Engine::computeInFlightFences[currentFrame]) != VK_SUCCESS) {
                throw std::runtime_error("failed to submit compute command buffer!");
            };
        }

        Engine::canDeleteObjectMutex.lock();
        Engine::gameObjectMutex.lock();
        Engine::meshMutex.lock();
        
        if(Engine::recreateVertexIndexBuffer && Engine::meshMap.size()!=0){

            Engine::updateVertexIndexBuffers();
            
        }else{
            Engine::verIndBufferComplete.release();
        }

        #ifndef EDITOR
            handleCommandBufferRecording(imageIndex);
        #else
            Engine::wasPlacedInThread = false;
        #endif

		Engine::verIndBufferComplete.acquire();

        handleObjectUpdates();

        #ifdef EDITOR
            handleDebugBuffers();
        #endif
        
        Engine::meshMutex.unlock();

        if(!Engine::wasPlacedInThread){
            Engine::commandPoolMutex.lock();
            BufferManager::recordCommandBuffer(Engine::commandBuffers[Engine::currentFrame], imageIndex,device,
            physicalDevice);
            Engine::commandPoolMutex.unlock();
        }

		Engine::commandBufferRecorded.acquire();
        
        Engine::gameObjectMutex.unlock();
        Engine::canDeleteObjectMutex.unlock();

        submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = 
        {
            Engine::computeFinishedSemaphores[currentFrame], 
            Engine::imageAvailableSemaphores[Engine::currentFrame]
        };

        VkPipelineStageFlags waitStages[] = 
        {
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };

        submitInfo.waitSemaphoreCount = 2;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &Engine::commandBuffers[Engine::currentFrame];

        VkSemaphore signalSemaphores[] = {Engine::renderFinishedSemaphores[Engine::currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        Engine::graphicsQueueMutex.lock();

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, Engine::inFlightFences[Engine::currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        Engine::graphicsQueueMutex.unlock();

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {Engine::swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        Engine::graphicsQueueMutex.lock();

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        Engine::graphicsQueueMutex.unlock();

        if (result == VK_ERROR_OUT_OF_DATE_KHR || Engine::framebufferResized) {
            SwapChainManager::recreateSwapChain(surface,physicalDevice,device,presentQueue);
            framebufferResized = false;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        Engine::currentFrame = (Engine::currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        Engine::frameCount++;
        Engine::lastFrameTime = std::chrono::system_clock::now();
    }

    void Engine::frameBufferResizeCallback(GLFWwindow* window, int width, int height){
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void Engine::updateGameObjects(){

        for(size_t i=0; i<Engine::meshBatches.size(); i++){
            delete Engine::meshBatches[i];
        }

        Engine::meshBatches.clear();

        VkDeviceSize bufferSize=0;

        Engine::meshSet.clear();
        Engine::textureIndices.clear();

        Engine::meshBatches.reserve(Engine::objectsByMesh.size());
        for(auto& meshName : Engine::objectsByMesh){
            Engine::meshBatches.push_back(new MeshBatch(meshName.first));
            Engine::meshSet.insert({meshName.first,Engine::meshBatches[Engine::meshBatches.size() - 1]});
        }

        GameObject* object = Engine::objectDQueue.head;

        while(true){
            Engine::textureMutex.lock();
            if (Engine::textureIndices.count(object->texturePath)==0) {
                Engine::textureIndices[object->texturePath] = Engine::meshSet[object->meshPath]->textures.size();
                Engine::meshSet[object->meshPath]->textures.push_back(&Engine::textureMap.at(object->texturePath));
            }
            Engine::textureMutex.unlock();

            object->update();

            object->updateInstanceInfo(Engine::textureIndices.at(object->texturePath));

            Engine::meshSet[object->meshPath]->instances.push_back(
                *(object->info)
            );
            Engine::meshSet[object->meshPath]->objects.push_back(object);

            bufferSize+=sizeof(InstanceInfo);

            if(object->next == nullptr){
                
                if(bufferSize>Engine::instanceBufferSize){
                    Engine::recreateInstanceBuffer=true;
                }

                //for(size_t i=0; i<Engine::meshBatches.size(); i++){
                    //std::cout<<Engine::meshBatches[i].meshPath<<" "<<Engine::meshBatches[i].objects.size()<<std::endl;
                //}
                
                object = Engine::objectDQueue.head;
                while(true){
                    if(object->moved){
                        object->checkCollisions();
                    }
        
                    if(object->next == nullptr){
                        break;
                    }
                    
                    object = object->next;
                }

                Engine::safeToMakeInstanceBuffer.release();

                return;
            }
            
            object = object->next;
        }
    }

    VkSampleCountFlagBits Engine::getMaxUsableSampleCount(VkPhysicalDevice& physicalDevice){
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    void Engine::initThreadPool(uint16_t poolSize, VkDevice& device, VkPhysicalDevice& physicalDevice
        , VkSurfaceKHR& surface){

        std::cout<<"\nThreads in pool: "<<poolSize<<"\n"<<std::endl;
        for (uint16_t i=0; i<poolSize; i++){
            WorkerThread* wt = new WorkerThread(device, physicalDevice,surface);

            Engine::threadPool[wt->id]=wt;
        }
    }

    void Engine::createUpdateTextureQueueJob(){
        UpdateTextureDeleteQueueJob* j = new UpdateTextureDeleteQueueJob();
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

        Engine::workInQueueSemaphore.release();
    }

    void Engine::updateMeshDataStructures(){

        UpdateMeshDataStructuresJob* j = new UpdateMeshDataStructuresJob();

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

		Engine::workInQueueSemaphore.release();
    }

    void Engine::createUpdateDescriptorQueueJob(){

        UpdateDescriptorDeleteQueueJob* j = new UpdateDescriptorDeleteQueueJob();
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

        Engine::workInQueueSemaphore.release();
    }

    void Engine::createUpdateObjDescrJob(Latch* l){

        UpdateGameObjectsAndDescriptorsJob* j = new UpdateGameObjectsAndDescriptorsJob();
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j->data.emplace_back(std::in_place_type<std::binary_semaphore*>,&Engine::descriptorsReadySemaphore);
        j->data.emplace_back(std::in_place_type<std::binary_semaphore*>,&Engine::safeToMakeInstanceBuffer);
        j->data.emplace_back(std::in_place_type<Latch*>, l);
        j->data.emplace_back(std::in_place_type<std::binary_semaphore*>, &Engine::setReady);

        Engine::jobQueue.push(j);

        Engine::workInQueueSemaphore.release();

        UpdateGameObjectsJob* j2 = new UpdateGameObjectsJob();
        j2->data.emplace_back(std::in_place_type<Latch*>, l);
        j2->data.emplace_back(std::in_place_type<std::binary_semaphore*>, &Engine::setReady);

        Engine::jobQueue.push(j2);

        Engine::workInQueueSemaphore.release();

    }

    void Engine::createVertexIndexBufferUpdateJob(){

        UpdateVertexIndexBufferJob* j = new UpdateVertexIndexBufferJob();
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j->data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);
        j->data.emplace_back(std::in_place_type<VkQueue*>, &graphicsQueue);

        Engine::jobQueue.push(j);

        Engine::workInQueueSemaphore.release();
    }

    void Engine::createInstanceBufferRemakeJob(){

        MakeInstanceBufferJob* j = new MakeInstanceBufferJob();
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j->data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);
        j->data.emplace_back(std::in_place_type<std::binary_semaphore*>,&Engine::instanceBufferReady);

        Engine::jobQueue.push(j);

        Engine::workInQueueSemaphore.release();
    }

    void Engine::createInstanceBufferUpdateJob(){

        UpdateInstanceBufferJob* j = new UpdateInstanceBufferJob();
        j->data.emplace_back((uint64_t)&Engine::currentFrame);

        Engine::jobQueue.push(j);

        Engine::workInQueueSemaphore.release();
    }

    void Engine::updateVertexIndexBuffers(){

        Engine::queueMutex.lock();

        if(Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){

            createVertexIndexBufferUpdateJob();

            Engine::queueMutex.unlock();

            return;
        }

        Engine::queueMutex.unlock();

        uint64_t size = BufferManager::remakeVertexIndexVectors(this->device);

        if( size >=Engine::indexVertexBufferSize)
        {
            BufferManager::createIndexVertexBuffer(this->device,this->physicalDevice,
                this->graphicsQueue, Engine::commandPool);

        }else{
            BufferManager::updateIndexVertexBuffer(this->device,this->physicalDevice,
                this->graphicsQueue, Engine::commandPool);
        }

        Engine::recreateVertexIndexBuffer=false;
		Engine::verIndBufferComplete.release();

    }

    void Engine::updateDescriptors(){
        if(Engine::descriptorSets.size()==0 || Engine::meshBatches.size()!=Engine::descriptorSets[0].size() || Engine::recreateDescriptors){

            Engine::queueMutex.lock();
            if(Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){
                
                DescriptorManager::recreateDescriptors(this->device);

                Engine::queueMutex.unlock();
            }else{
                
                Engine::queueMutex.unlock();

                recreateDescriptorSetsAndPool(device,&Engine::descriptorsReadySemaphore);
            }

            Engine::recreateDescriptors=false;

        }else{
            Engine::descriptorsReadySemaphore.release();
        }

    }

    void Engine::checkInstanceBufferForUpdates(){
        if(Engine::recreateInstanceBuffer){

            BufferManager::recreateInstanceBuffers(this->device,this->physicalDevice);

            recreateInstanceBuffer=false;

        }else{
            Engine::instanceBufferReady.release();
        }
    }

    void Engine::createRecordCommandBufferJob(uint32_t imageIndex){

        RecordCommandBufferJob* j = new RecordCommandBufferJob();
        j->data.emplace_back(std::in_place_type<VkCommandBuffer*>, &Engine::commandBuffers[Engine::currentFrame]);
        j->data.emplace_back(std::in_place_type<uint32_t>, imageIndex);
        j->data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j->data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);

        Engine::jobQueue.push(j);

        Engine::workInQueueSemaphore.release();
    }

    void Engine::handleCommandBufferRecording(uint32_t imageIndex){

        Engine::queueMutex.lock();
        if(Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){
            
            Engine::wasPlacedInThread=true;

            createRecordCommandBufferJob(imageIndex);

            Engine::queueMutex.unlock();
        }else{
            Engine::queueMutex.unlock();

            Engine::wasPlacedInThread=false;
        }
    }

    void Engine::objectLoadingTest(std::chrono::system_clock::time_point frameZeroTime){
        for(int i=0; i<40; i++){
            /*GameObject::createObjectThreaded("../textures/statue.jpg",
                "../models/stanford_sphere.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );

            GameObject::createObjectThreaded("../textures/angel.jpg", 
                "../models/cube.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );

            GameObject::createObjectThreaded("../textures/viking_room.png", 
                "../models/viking_room.obj", 
                device, 
                physicalDevice, 
                graphicsQueue
            );*/
        }

        if(Engine::frameCount%1300==0 && Engine::frameCount>0){ //About 156.000 objects
            std::cout<<"====== FRAME "<<Engine::frameCount<<" ======"<<std::endl;
            Engine::gameObjectMutex.lock();
            std::cout<<Engine::objectDQueue.size<<" objects loaded"<<std::endl;

            auto finalTime = std::chrono::system_clock::now();

            std::chrono::duration<double> deltaSec = finalTime - frameZeroTime;
            std::chrono::duration<double, std::milli> deltaMs = finalTime - frameZeroTime;

            std::cout << "Delta time: " << deltaSec.count() << " seconds\n";
            std::cout << "Delta time: " << deltaMs.count() << " milliseconds\n";
        }
    }

    void Engine::killThreads(){

        Engine::queueMutex.lock();

        while(!Engine::jobQueue.empty()){
            Engine::jobQueue.pop();
        }

        while(!Engine::deferredJobQueue.empty()){
            Engine::deferredJobQueue.pop();
        }

        for(size_t i=0; i<Engine::threadPool.size(); i++){

            PrepareForJoinJob* j = new PrepareForJoinJob();
            j->data.emplace_back(std::in_place_type<VkDevice*>, &device);

            Engine::jobQueue.push(j);
        }
        
        Engine::queueMutex.unlock();

        for (const auto& pair : Engine::threadPool) {
            pair.second->alive = false;
        }
        
        for (size_t i=0; i<Engine::threadPool.size(); i++) {
            Engine::workInQueueSemaphore.release();
        }
        
        int j=0;
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

    int Engine::createLinuxDesktopEntry(const char* argv0){
        
        #ifdef __linux__
            const char* home = std::getenv("HOME");
            if (!home) return 1;

            std::string desktopDir = std::string(home) + "/.local/share/applications/";
            std::filesystem::create_directories(desktopDir);

            std::string desktopFile = desktopDir + "prometheus.desktop";
            if (std::filesystem::exists(desktopFile)) {
                std::cout << "Desktop entry already exists, skipping creation.\n";
                return 0;
            }

            // Get full executable path
            char exePath[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", exePath, sizeof(exePath) - 1);
            if (len != -1) exePath[len] = '\0';
            else strlcpy(exePath, argv0, sizeof(exePath));
            exePath[sizeof(exePath) - 1] = '\0';

            // Build absolute paths
            std::string execPath = std::filesystem::absolute(exePath).string();
            std::string iconPath = (Engine::exeDir / "../textures/logo/logo-144.png").lexically_normal().c_str();

            if (!std::filesystem::exists(iconPath)) {
                std::cerr << "Warning: icon file not found at " << iconPath << std::endl;
                return 2;
            }

            // Create desktop entry
            std::ofstream file(desktopDir + "prometheus.desktop");
            if (!file.is_open()) {
                std::cerr << "Failed to create desktop entry!" << std::endl;
                return 3;
            }

            file << "[Desktop Entry]\n"
                << "Name=Prometheus\n"
                << "Exec=" << execPath << "\n"
                << "Icon=" << iconPath << "\n"
                << "Type=Application\n"
                << "StartupWMClass=Prometheus\n"
                << "Terminal=false\n"
                << "Categories=Development;Game;\n";
            file.close();

            // Refresh desktop database (optional)
            return system(("update-desktop-database " + desktopDir).c_str());
        #endif  

        return 0;
    }

    void Engine::createSpatialHash(glm::vec3 maxCoords, glm::vec3 minCoords){

        float xStep = (maxCoords.x - minCoords.x)/Engine::spatialHash.size();
        float yStep = (maxCoords.y - minCoords.y)/Engine::spatialHash.size();
        float zStep = (maxCoords.z - minCoords.z)/Engine::spatialHash.size();

        Engine::cellSize = glm::vec3(xStep, yStep, zStep);

        glm::vec3 min = glm::vec3(minCoords);
        glm::vec3 max = glm::vec3(minCoords.x + xStep, minCoords.y + yStep, minCoords.z + zStep);

        for(size_t i=0; i<Engine::spatialHash.size(); i++){
            for(size_t j=0; j<Engine::spatialHash.size(); j++){
                for(size_t k=0; k<Engine::spatialHash.size(); k++){

                    Engine::spatialHash[i][j][k] = new Cell(min, max);
                    min.z += zStep;
                    max.z += zStep;
                }

                min.y += yStep;
                max.y += yStep;

                min.z = minCoords.z;
                max.z = minCoords.z + zStep;
            }

            min.x += xStep;
            max.x += xStep;

            min.y = minCoords.y;
            max.y = minCoords.y + yStep;

            min.z = minCoords.z;
            max.z = minCoords.z + xStep;
        }
    }

    void Engine::drawSpatialHash(){

        for(uint8_t i=0; i<Engine::spatialHash.size(); i++){
            for(uint8_t j=0; j<Engine::spatialHash.size(); j++){
                for(uint8_t k=0; k<Engine::spatialHash.size(); k++){

                    if(Engine::spatialHash[i][j][k]->objects.size()!=0){
                        Engine::spatialHash[i][j][k]->drawSelf();
                    }
                }
            }
        }
    }

    void Engine::insertToHash(GameObject* obj){

        glm::vec3 maxBounds = obj->getCenter();
        maxBounds.x += obj->radius;
        maxBounds.y += obj->radius;
        maxBounds.z += obj->radius;

        glm::vec3 minBounds = obj->getCenter();
        minBounds.x -= obj->radius;
        minBounds.y -= obj->radius;
        minBounds.z -= obj->radius;

        int minX = static_cast<int>(std::floor(minBounds.x / Engine::cellSize.x)) + 25;
        int minY = static_cast<int>(std::floor(minBounds.y / Engine::cellSize.y)) + 25;
        int minZ = static_cast<int>(std::floor(minBounds.z / Engine::cellSize.z)) + 25;
        
        int maxX = static_cast<int>(std::floor(maxBounds.x / Engine::cellSize.x)) + 25;
        int maxY = static_cast<int>(std::floor(maxBounds.y / Engine::cellSize.y)) + 25;
        int maxZ = static_cast<int>(std::floor(maxBounds.z / Engine::cellSize.z)) + 25;

        int indexX = static_cast<int>(std::floor(obj->center.x / Engine::cellSize.x)) + 25;
        int indexY = static_cast<int>(std::floor(obj->center.y / Engine::cellSize.y)) + 25;
        int indexZ = static_cast<int>(std::floor(obj->center.z / Engine::cellSize.z)) + 25;

        Engine::spatialHash[indexX][indexY][indexZ]->objectMutex.lock();
        Engine::spatialHash[indexX][indexY][indexZ]->objects.insert(obj);
        Engine::spatialHash[indexX][indexY][indexZ]->objectMutex.unlock();
        obj->cells.insert(Engine::spatialHash[indexX][indexY][indexZ]);

        for (int x = minX; x <= maxX; x++) {
            for (int y = minY; y <= maxY; y++) {
                for (int z = minZ; z <= maxZ; z++) {
                    
                    if(obj->cells.count(Engine::spatialHash[x][y][z])){
                        continue;
                    }

                    if(Atlas::Collision::checkOBBtoOBB(Engine::spatialHash[x][y][z]->getBasicAxes(),
                        obj->transform.getBasicAxes(), Engine::spatialHash[x][y][z]->edges,
                        obj->getWorldHitpoints()
                    )){
                        Engine::spatialHash[x][y][z]->objectMutex.lock();
                        Engine::spatialHash[x][y][z]->objects.insert(obj);
                        Engine::spatialHash[x][y][z]->objectMutex.unlock();
                        obj->cells.insert(Engine::spatialHash[x][y][z]);
                    }
                }
            }
        }
    }

    void Engine::handleDebugBuffers(){
        if(Engine::debugVertices.size()!=0){
            uint64_t size = (sizeof(Engine::debugVertices[0]) * Engine::debugVertices.size())+(sizeof(Engine::debugIndices[0]) * Engine::debugIndices.size());

            if(size >= Engine::debugIndexVertexBufferSize){

                if (Engine::debugIndexVertexBuffer != VK_NULL_HANDLE) {
                    vkDestroyBuffer(device, Engine::debugIndexVertexBuffer, nullptr);
                }
                if (Engine::debugIndexVertexBufferMemory != VK_NULL_HANDLE) {
                    vkFreeMemory(device, Engine::debugIndexVertexBufferMemory, nullptr);
                }

                BufferManager::createDebugIndexVertexBuffer(this->device,this->physicalDevice,
                this->graphicsQueue, Engine::commandPool);

            }else{
                BufferManager::updateDebugIndexVertexBuffer(this->device,this->physicalDevice,
                    this->graphicsQueue, Engine::commandPool);
            }
        }

        Engine::debugBuffersReady.release();
    }

    void Engine::handleObjectUpdates(){

        if(Engine::objectDQueue.size==0){
            Engine::descriptorsReadySemaphore.release();
			Engine::instanceBufferReady.release();

            return;
        }

        std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
        std::chrono::duration<double> delta = currentTime - Engine::lastUpdateTime;

        auto timeToUpdate = Engine::updateTime - delta.count();

        if(timeToUpdate > 0.0){

            Engine::descriptorsReadySemaphore.release();
            Engine::instanceBufferReady.release();
            
            return;

        }

        if(Engine::objectDQueue.size < 2){
            Engine::updateGameObjects();
            Engine::updateDescriptors();
        }else{

            if(Engine::threadsAvailable.getValue()>1 && Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){
                Latch* l = new Latch(1);
                createUpdateObjDescrJob(l);
            }else{
    
                Engine::updateGameObjects();
        
                Engine::updateDescriptors();
            }
        }

        Engine::safeToMakeInstanceBuffer.acquire();

        Engine::checkInstanceBufferForUpdates();
    }

    void Engine::createLightSource(glm::vec4 pos, glm::vec4 color, float intensity){

        new UniformBufferObject(pos, color, intensity);
    }

    void Engine::updateLightSources(){

        UBOContainer* light = Engine::lights.head;

        for(size_t i=0; i<Engine::lights.size; i++){

            light->ubo->update();

            light = light->next;
        }
    }

    void Engine::printJobQueue() {
        std::cout << "Current Job Queue ("<<Engine::threadsAvailable.getValue()<<"threads available):" << std::endl;
        std::queue<Job*> tempQueue = Engine::jobQueue;

		if (tempQueue.empty()) {
            std::cout << "[Empty]" << std::endl;
            return;
        }

        while (!tempQueue.empty()) {
            Job* j = tempQueue.front();
			std::cout << "[" << j->name() << "] ";
            tempQueue.pop();
        }

        std::cout<<std::endl;
    }

    void Engine::updateParticleSSBOs() {

		for (size_t i = 0; i < Engine::shaderStorageBuffers.size(); i++) {
            if (Engine::shaderStorageBuffers[i] != VK_NULL_HANDLE) {
                vkDestroyBuffer(device, Engine::shaderStorageBuffers[i], nullptr);
                vkFreeMemory(device, Engine::shaderStorageBuffersMemories[i], nullptr);
            }
        }

        BufferManager::createSSBOs(this->device, this->physicalDevice, Engine::shaderStorageBuffers,
            Engine::shaderStorageBufferSize, Engine::shaderStorageBuffersMemories, Engine::particles, this->graphicsQueue);
    }

    void Engine::remakeComputeDescriptorSetsAndPool() {

        Engine::graphicsQueueMutex.lock();
        vkDeviceWaitIdle(device);

        Engine::graphicsQueueMutex.unlock();

        Engine::descriptorQueuedMutex.lock();
        Engine::descriptorDeleteQueue.push_back(std::move(Engine::computePool));
        Engine::framesSinceDescriptorQueuedForDeletion.push_back(0);

        Engine::descriptorQueuedMutex.unlock();

        DescriptorManager::createComputeDescriptorPool(device);
        DescriptorManager::createComputeDescriptorSets(device);
    }
}