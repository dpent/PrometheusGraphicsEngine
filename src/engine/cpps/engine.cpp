#include "../headers/engine.h"
#include "../headers/deviceManager.h"
#include "../headers/swapChainManager.h"
#include "../headers/graphicsPipelineManager.h"
#include "../headers/renderPassManager.h"
#include "../headers/bufferManager.h"
#include "../headers/syncManager.h"
#include <semaphore.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "../headers/descriptorManager.h"
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <unistd.h> 
#include <string.h>
#include <chrono>
#include "../../threads/headers/descriptorOperations.h"
#include "../headers/cell.h"

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
VkPipelineLayout Engine::pipelineLayout;
VkRenderPass Engine::renderPass;
VkPipeline Engine::graphicsPipeline;
VkPipeline Engine::preGraphicsPipeline;
VkPipelineLayout Engine::preGraphicsLayout;
VkPipeline Engine::debugPipeline;
VkPipelineLayout Engine::debugPipelineLayout;

VkCommandPool Engine::commandPool;
std::vector<VkCommandBuffer> Engine::commandBuffers;

std::vector<VkFramebuffer> Engine::swapChainFramebuffers;

std::vector<VkSemaphore> Engine::imageAvailableSemaphores;
std::vector<VkSemaphore> Engine::renderFinishedSemaphores;
std::vector<VkFence> Engine::inFlightFences;
sem_t Engine::descriptorsReadySemaphore;
sem_t Engine::safeToMakeInstanceBuffer;
sem_t Engine::verIndBufferComplete;
sem_t Engine::instanceBufferReady;
sem_t Engine::commandBufferRecorded;
std::mutex Engine::gameObjectMutex;
std::mutex Engine::canDeleteObjectMutex;
std::mutex Engine::textureMutex;
std::mutex Engine::textureQueuedMutex;
std::mutex Engine::graphicsQueueMutex;
std::mutex Engine::commandPoolMutex;
std::mutex Engine::meshMutex;
std::mutex Engine::descriptorQueuedMutex;


uint32_t Engine::currentFrame = 0;

bool Engine::framebufferResized = false;

std::vector<Vertex> Engine::vertices;
std::vector<uint32_t> Engine::indices;

std::vector<Vertex> Engine::debugVertices;
std::vector<uint32_t> Engine::debugIndices;

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
std::vector<VkDescriptorSet> Engine::descriptorSets;
std::list<VkDescriptorPool> Engine::descriptorDeleteQueue;
std::list<int> Engine::framesSinceDescriptorQueuedForDeletion;

//std::unordered_map<uint64_t,GameObject*> Engine::gameObjectMap;
DoubleEndedQueue<GameObject*> Engine::objectDQueue;

VkPhysicalDeviceProperties Engine::physicalDeviceProperties;
VkPhysicalDeviceFeatures Engine::physicalDeviceFeatures;

std::unordered_map<std::string, Texture> Engine::textureMap;
std::unordered_map<std::string, std::vector<Texture>> Engine::texturesQueuedForDeletion;
std::unordered_map<std::string, std::vector<int>> Engine::framesSinceTextureQueuedForDeletion;

std::unordered_map<std::string,Mesh> Engine::meshMap;
std::unordered_map<std::string,bool> Engine::meshesLoading;
std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>> Engine::objectsByMesh;
std::vector<MeshBatch> Engine::meshBatches;

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
std::queue<Job> Engine::jobQueue;
std::queue<Job> Engine::deferredJobQueue;
std::mutex Engine::queueMutex;
sem_t Engine::workInQueueSemaphore;

uint64_t Engine::frameCount=0;

SafeUint16_t Engine::threadsAvailable = SafeUint16_t(std::thread::hardware_concurrency()-1);

bool Engine::wasPlacedInThread=false;

std::filesystem::path Engine::exeDir = std::filesystem::canonical("/proc/self/exe").parent_path();

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

Cell* Engine::spatialHash;

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

        sem_init(&Engine::descriptorsReadySemaphore,0,0);
        sem_init(&Engine::safeToMakeInstanceBuffer,0,0);
        sem_init(&Engine::verIndBufferComplete,0,0);
        sem_init (&Engine::commandBufferRecorded,0,0);

        Engine::pressed.resize(349, false);
        Engine::camera.updateCameraVectors();

        InstanceManager::createInstance(this->instance);
        InstanceManager::setupDebugMessenger(this->instance,this->debugMessenger);

        Engine::createSurface();

        DeviceManager::pickPhysicalDevice(this->instance,this->physicalDevice, this->surface);
        DeviceManager::createLogicalDevice(this->physicalDevice, this->device, this->graphicsQueue,this->presentQueue, this->surface);

        vkGetPhysicalDeviceProperties(physicalDevice, &Engine::physicalDeviceProperties); //We will use them for anisotropic filtering etc later on
        vkGetPhysicalDeviceFeatures(physicalDevice, &Engine::physicalDeviceFeatures);

        Engine::initThreadPool(std::thread::hardware_concurrency()-1,this->device, this->physicalDevice,
        this->surface);

        SwapChainManager::createSwapChain(this->surface,this->physicalDevice,this->device, Engine::swapChain);
        SwapChainManager::createImageViews(this->device);

        RenderPassManager::createRenderPass(this->device, this->physicalDevice);

        DescriptorManager::createDescriptorSetLayout(this->device);

        GraphicsPipelineManager::createGraphicsPipeline(this->device);

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

        TextureManager::createSolidColorTextureFile("../textures/babyBlue.png",137,207,240);
        TextureManager::createSolidColorTextureFile("../textures/red.png",255,0,0);
        TextureManager::createSolidColorTextureFile("../textures/green.png",0,255,0);
        TextureManager::createSolidColorTextureFile("../textures/magenta.png",255,0,255);
        
        GameObject::createObjectThreaded("../textures/babyBlue.png", 
            "../models/stanford_dragon.obj", 
            device, 
            physicalDevice, 
            graphicsQueue
        );

        GameObject::createObjectThreaded("../textures/red.png", 
            "../models/stanford_dragon.obj", 
            device, 
            physicalDevice, 
            graphicsQueue
        );

        GameObject::createObjectThreaded("../textures/green.png", 
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
        //BufferManager::createUniformBuffers(this->device,this->physicalDevice);

        Engine::instanceBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBufferMemories.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBuffersMapped.resize(Engine::MAX_FRAMES_IN_FLIGHT);

        SyncManager::createSyncObjects(this->device);

    }

    void Engine::mainLoop() {

        InputManager::initInputMode(false,false,false,false,false,Engine::window);

        glfwSetKeyCallback(Engine::window, InputManager::keyCallBack);
        glfwSetCursorEnterCallback(Engine::window, InputManager::mouseEnterCallBack);
        glfwSetMouseButtonCallback(Engine::window, InputManager::mouseButtonCallBack);
        glfwSetScrollCallback(Engine::window, InputManager::mouseScrollCallBack);
        glfwSetCursorPosCallback(Engine::window, InputManager::cursorPosCallBack);

        Engine::commandPoolMutex.lock();
        WindowManager::initGUI(instance,graphicsQueue,device,physicalDevice);
        Engine::commandPoolMutex.unlock();

        Engine::spatialHash = new Cell(glm::vec3(-100.0f,-100.0f,-100.0f),glm::vec3(100.0f,100.0f,100.0f));
        //spatialHash->split();
        //auto frameZeroTime = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(Engine::window)) {
            glfwPollEvents();

            InputManager::consumeInput(Engine::window);

            if(Engine::updateCameraVectors){
                Engine::camera.updateCameraVectors();
                Engine::updateCameraVectors = false;
            }

            WindowManager::startNewFrame();
            Engine::spatialHash->drawAll();

            drawFrame();

            //objectLoadingTest(frameZeroTime);
            
            createUpdateTextureQueueJob();
            createUpdateDescriptorQueueJob();
            
            #ifdef EDITOR
                Engine::debugVertices.clear();
                Engine::debugIndices.clear();
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
                
                if(object->next == nullptr){
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

        WindowManager::cleanup();

        vkDestroyDescriptorPool(device, Engine::imGUIPool, nullptr);

        SwapChainManager::cleanupSwapChain(device);

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //vkDestroyBuffer(device, Engine::uniformBuffers[i], nullptr);
            //vkFreeMemory(device, Engine::uniformBuffersMemory[i], nullptr);
        //}

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
        }

        vkDestroyCommandPool(device, commandPool, nullptr);

        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        vkDestroyPipeline(device, preGraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(device, preGraphicsLayout, nullptr);
        vkDestroyPipeline(device, debugPipeline, nullptr);
        vkDestroyPipelineLayout(device, debugPipelineLayout, nullptr);

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

        Engine::canDeleteObjectMutex.lock();
        Engine::gameObjectMutex.lock();
        Engine::meshMutex.lock();

        if(Engine::recreateVertexIndexBuffer && Engine::meshMap.size()!=0){

            Engine::updateVertexIndexBuffers();
            
        }else{
            sem_post(&Engine::verIndBufferComplete);
        }

        #ifdef EDITOR

        #endif

        sem_wait(&Engine::verIndBufferComplete);

        if(Engine::objectDQueue.size!=0){

            std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
            std::chrono::duration<double> delta = currentTime - Engine::lastUpdateTime;

            auto timeToUpdate = Engine::updateTime - delta.count();

            if(timeToUpdate <= 0.0){

                Engine::updateGameObjects();
    
                Engine::updateDescriptors();
    
                sem_wait(&Engine::safeToMakeInstanceBuffer);
    
                Engine::checkInstanceBufferForUpdates();
            }else{
                sem_post(&Engine::descriptorsReadySemaphore);
                sem_post(&Engine::instanceBufferReady);
            }

            
            
        }else{
            
            sem_post(&Engine::descriptorsReadySemaphore);
            sem_post(&Engine::instanceBufferReady);
        }

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

        //handleCommandBufferRecording(imageIndex);
        
        Engine::meshMutex.unlock();
        if(!Engine::wasPlacedInThread){
            Engine::commandPoolMutex.lock();
            BufferManager::recordCommandBuffer(Engine::commandBuffers[Engine::currentFrame], imageIndex,device,
            physicalDevice);
            Engine::commandPoolMutex.unlock();
        }

        sem_wait(&Engine::commandBufferRecorded);
        
        Engine::gameObjectMutex.unlock();
        Engine::canDeleteObjectMutex.unlock();

        //BufferManager::updateUniformBuffer(Engine::currentFrame);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {Engine::imageAvailableSemaphores[Engine::currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
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

        Engine::meshBatches.clear();

        int i=0;
        VkDeviceSize bufferSize=0;
        for (auto& [meshName, innerMap] : Engine::objectsByMesh) {

            uint64_t currIndex=0;
            std::unordered_map<std::string,uint64_t> textureIndices;
            Engine::meshBatches.push_back(MeshBatch(meshName));

            for (auto& [id, objPtr] : innerMap) {

                Engine::textureMutex.lock();
                if (textureIndices.find(objPtr->texturePath) == textureIndices.end()) {
                    Engine::meshBatches[Engine::meshBatches.size()-1].textures.push_back(&Engine::textureMap.at(objPtr->texturePath));
                    textureIndices[objPtr->texturePath] = currIndex;
                    currIndex++; 
                }
                Engine::textureMutex.unlock();

                objPtr->update();

                objPtr->updateInstanceInfo(textureIndices.at(objPtr->texturePath));

                Engine::meshBatches[Engine::meshBatches.size()-1].instances.push_back(
                    *(objPtr->info)
                );
                Engine::meshBatches[Engine::meshBatches.size()-1].objects.push_back(objPtr);
                i++;
            }
            if(Engine::meshBatches[Engine::meshBatches.size()-1].objects.size()==0){
                Engine::meshBatches.pop_back();
            }
            bufferSize+=sizeof(InstanceInfo) * Engine::meshBatches[Engine::meshBatches.size()-1].instances.size();
        }

        if(bufferSize>Engine::instanceBufferSize){
            Engine::recreateInstanceBuffer=true;
        }

        sem_post(&Engine::safeToMakeInstanceBuffer);
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

        sem_init(&(Engine::workInQueueSemaphore),0,0);
        std::cout<<"\nThreads in pool: "<<poolSize<<"\n"<<std::endl;
        for (uint16_t i=0; i<poolSize; i++){
            WorkerThread* wt = new WorkerThread(device, physicalDevice,surface);

            Engine::threadPool[wt->id]=wt;
        }
    }

    std::vector<std::queue<Job*>> Engine::batchJobs(){
        std::vector<std::queue<Job*>> batches;
        std::unordered_map<std::string,std::queue<Job*>> batchMap;

        while(!jobQueue.empty()){
            Job* j = new Job(jobQueue.front());
            //std::cout<<j->opId<<std::endl;

            switch (j->opId){
                
                case 0:
                case 1:
                case 2:
                case 3:

                    batchMap["0123"].push(j);
                    break;
                
                case 4:
                    batchMap["4"].push(j);
                    break;
                
                case 5:

                    batchMap["5"].push(j);
                    break;

                case 6:
                    batchMap["6"].push(j);
                    break;
                default:
                    break;

            }
            jobQueue.pop();
        }

        batches.reserve(batchMap.size());

        for (auto& pair : batchMap) {
            batches.push_back(pair.second);
        }

        return batches;
    }

    void Engine::createUpdateTextureQueueJob(){
        Job j = Job(UPDATE_TEXTURE_DELETE_QUEUE);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

        sem_post(&(Engine::workInQueueSemaphore));
    }

    void Engine::updateMeshDataStructures(){
        Job j = Job(UPDATE_MESH_DATA_STRUCTURES);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

        sem_post(&(Engine::workInQueueSemaphore));
    }

    void Engine::createUpdateDescriptorQueueJob(){

        Job j = Job(UPDATE_DESCRIPTOR_DELETE_QUEUE);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);

        Engine::queueMutex.lock();
        Engine::jobQueue.push(j);
        Engine::queueMutex.unlock();

        sem_post(&(Engine::workInQueueSemaphore));
    }

    void Engine::createUpdateObjDescrJob(){

        Job j = Job(UPDATE_OBJECTS_AND_DESCRIPTORS);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j.data.emplace_back(std::in_place_type<sem_t*>,&Engine::descriptorsReadySemaphore);
        j.data.emplace_back(std::in_place_type<sem_t*>,&Engine::safeToMakeInstanceBuffer);

        Engine::jobQueue.push(j);

        sem_post(&Engine::workInQueueSemaphore);

    }

    void Engine::createVertexIndexBufferUpdateJob(){

        Job j = Job(UPDATE_VERTEX_INDEX_BUFFER);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j.data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);
        j.data.emplace_back(std::in_place_type<VkQueue*>, &graphicsQueue);

        Engine::jobQueue.push(j);

        sem_post(&Engine::workInQueueSemaphore);
    }

    void Engine::createInstanceBufferRemakeJob(){
        Job j = Job(MAKE_INSTANCE_BUFFER);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j.data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);
        j.data.emplace_back(std::in_place_type<sem_t*>,&Engine::instanceBufferReady);

        Engine::jobQueue.push(j);

        sem_post(&Engine::workInQueueSemaphore);
    }

    void Engine::createInstanceBufferUpdateJob(){
        Job j = Job(UPDATE_INSTANCE_BUFFER);
        j.data.emplace_back((uint64_t)&Engine::currentFrame);

        Engine::jobQueue.push(j);

        sem_post(&Engine::workInQueueSemaphore);
    }

    void Engine::updateVertexIndexBuffers(){

        Engine::queueMutex.lock();

        if(Engine::threadsAvailable.getValue()!=0 && Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){

            createVertexIndexBufferUpdateJob();

            Engine::queueMutex.unlock();
        }else{

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
            sem_post(&Engine::verIndBufferComplete);
        }
    }

    void Engine::updateDescriptors(){
        if(Engine::meshBatches.size()!=Engine::descriptorSets.size() || Engine::recreateDescriptors){

            Engine::queueMutex.lock();
            if(Engine::threadsAvailable.getValue()!=0 && Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){
                
                DescriptorManager::recreateDescriptors(this->device);

                Engine::queueMutex.unlock();
            }else{
                
                Engine::queueMutex.unlock();

                recreateDescriptorSetsAndPool(device,&Engine::descriptorsReadySemaphore);
            }

            Engine::recreateDescriptors=false;

        }else{
            sem_post(&Engine::descriptorsReadySemaphore);
        }

    }

    void Engine::checkInstanceBufferForUpdates(){
        if(Engine::recreateInstanceBuffer){

            BufferManager::recreateInstanceBuffers(this->device,this->physicalDevice);

            recreateInstanceBuffer=false;

        }else{
            sem_post(&Engine::instanceBufferReady);
        }
    }

    void Engine::createRecordCommandBufferJob(uint32_t imageIndex){

        Job j = Job(RECORD_COMMAND_BUFFER);
        j.data.emplace_back(std::in_place_type<VkCommandBuffer*>, &Engine::commandBuffers[Engine::currentFrame]);
        j.data.emplace_back(std::in_place_type<uint32_t>, imageIndex);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j.data.emplace_back(std::in_place_type<VkPhysicalDevice*>, &physicalDevice);

        Engine::jobQueue.push(j);

        sem_post(&Engine::workInQueueSemaphore);
    }

    void Engine::handleCommandBufferRecording(uint32_t imageIndex){

        Engine::queueMutex.lock();
        if(Engine::threadsAvailable.getValue()!=0 && Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){
            
            Engine::wasPlacedInThread=true;

            createRecordCommandBufferJob(imageIndex);

            Engine::queueMutex.unlock();
        }else{
            Engine::queueMutex.unlock();

            Engine::wasPlacedInThread=false;
        }
    }

    void Engine::objectLoadingTest(std::chrono::_V2::system_clock::time_point frameZeroTime){
        for(int i=0; i<40; i++){
            GameObject::createObjectThreaded("../textures/statue.jpg", 
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
            );
        }

        if(Engine::frameCount%1300==0 && Engine::frameCount>0){ //About 156.000 objects
            std::cout<<"====== FRAME "<<Engine::frameCount<<" ======"<<std::endl;
            Engine::gameObjectMutex.lock();
            std::cout<<Engine::objectDQueue.size<<" objects loaded"<<std::endl;

            auto finalTime = std::chrono::high_resolution_clock::now();
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
            Job j = Job(PREPARE_FOR_JOIN);
            j.data.emplace_back(std::in_place_type<VkDevice*>, &device);

            Engine::jobQueue.push(j);
        }
        
        Engine::queueMutex.unlock();

        for (const auto& pair : Engine::threadPool) {
            pair.second->alive = false;
        }
        
        for (size_t i=0; i<Engine::threadPool.size(); i++) {
            sem_post(&(Engine::workInQueueSemaphore));
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
    }
}