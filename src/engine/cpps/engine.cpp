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
#include <chrono>

using namespace Prometheus;

//DEFINE STATIC VARIABLES BEFORE VULKAN INIT
GLFWwindow* Engine::window = nullptr;
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

VkCommandPool Engine::commandPool;
std::vector<VkCommandBuffer> Engine::commandBuffers;

std::vector<VkFramebuffer> Engine::swapChainFramebuffers;

std::vector<VkSemaphore> Engine::imageAvailableSemaphores;
std::vector<VkSemaphore> Engine::renderFinishedSemaphores;
std::vector<VkFence> Engine::inFlightFences;
sem_t Engine::descriptorsReadySemaphore;
sem_t Engine::safeToMakeInstanceBuffer;
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

VkBuffer Engine::indexVertexBuffer= nullptr;
VkDeviceMemory Engine::indexVertexBufferMemory= nullptr;
uint64_t Engine::indexVertexBufferSize = 0;

std::vector<VkBuffer> Engine::instanceBuffers;
std::vector<VkDeviceMemory> Engine::instanceBufferMemories;
std::vector<void*> Engine::instanceBuffersMapped;
uint64_t Engine::instanceBufferSize = 0;

VkDeviceSize Engine::indexOffset=0;

std::vector<VkBuffer> Engine::uniformBuffers;
std::vector<VkDeviceMemory> Engine::uniformBuffersMemory;
std::vector<void*> Engine::uniformBuffersMapped;

VkDescriptorPool Engine::descriptorPool;
std::vector<VkDescriptorSet> Engine::descriptorSets;
std::list<VkDescriptorPool> Engine::descriptorDeleteQueue;
std::list<int> Engine::framesSinceDescriptorQueuedForDeletion;

glm::mat4 Engine::model;
glm::mat4 Engine::view;
glm::mat4 Engine::proj;

std::unordered_map<uint64_t,GameObject*> Engine::gameObjectMap;

VkPhysicalDeviceProperties Engine::physicalDeviceProperties;
VkPhysicalDeviceFeatures Engine::physicalDeviceFeatures;

std::unordered_map<std::string, Texture> Engine::textureMap;
std::unordered_map<std::string,std::vector<uint64_t>> Engine::objectIdsByTexture;
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

namespace Prometheus{
    void Engine::run() {
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
    }

    void Engine::initVulkan() {

        sem_init(&Engine::descriptorsReadySemaphore,0,0);
        sem_init(&Engine::safeToMakeInstanceBuffer,0,0);

        Engine::initThreadPool(std::thread::hardware_concurrency()-1);

        InstanceManager::createInstance(this->instance);
        InstanceManager::setupDebugMessenger(this->instance,this->debugMessenger);

        Engine::createSurface();

        DeviceManager::pickPhysicalDevice(this->instance,this->physicalDevice, this->surface);
        DeviceManager::createLogicalDevice(this->physicalDevice, this->device, this->graphicsQueue,this->presentQueue, this->surface);

        vkGetPhysicalDeviceProperties(physicalDevice, &Engine::physicalDeviceProperties); //We will use them for anisotropic filtering etc later on
        vkGetPhysicalDeviceFeatures(physicalDevice, &Engine::physicalDeviceFeatures);

        SwapChainManager::createSwapChain(this->surface,this->physicalDevice,this->device, Engine::swapChain);
        SwapChainManager::createImageViews(this->device);

        RenderPassManager::createRenderPass(this->device, this->physicalDevice);

        DescriptorManager::createDescriptorSetLayout(this->device);

        GraphicsPipelineManager::createGraphicsPipeline(this->device);

        BufferManager::createColorResources(this->device,this->physicalDevice);
        BufferManager::createDepthResources(this->device,this->physicalDevice);
        BufferManager::createFrameBuffers(this->device);
        BufferManager::createCommandPool(this->physicalDevice, this->surface,this->device);
        BufferManager::createCommandBuffers(this->device);

        for(int i=0; i<10; i++){
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

        //BufferManager::createUniformBuffers(this->device,this->physicalDevice);

        Engine::instanceBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBufferMemories.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBuffersMapped.resize(Engine::MAX_FRAMES_IN_FLIGHT);

        SyncManager::createSyncObjects(this->device);

    }

    void Engine::mainLoop() {
        auto frameZeroTime = std::chrono::high_resolution_clock::now();
        while (!glfwWindowShouldClose(Engine::window)) {
            glfwPollEvents();
            drawFrame();

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

            if(Engine::frameCount%1300==0 && Engine::frameCount>0){
                std::cout<<"====== FRAME "<<Engine::frameCount<<" ======"<<std::endl;
                Engine::gameObjectMutex.lock();
                std::cout<<Engine::gameObjectMap.size()<<" objects loaded"<<std::endl;
                Engine::gameObjectMutex.unlock();

                auto finalTime = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> deltaSec = finalTime - frameZeroTime;
                std::chrono::duration<double, std::milli> deltaMs = finalTime - frameZeroTime;

                std::cout << "Delta time: " << deltaSec.count() << " seconds\n";
                std::cout << "Delta time: " << deltaMs.count() << " milliseconds\n";
                /*if(Engine::gameObjectMap.size()>3){

                    int count=0;
                    for (const auto& pair : Engine::gameObjectMap) {
                        GameObject::deleteObjectThreaded(device,pair.second->id);
                        count++;
                        if(count==3){
                            break;
                        }
                    }
                    Engine::gameObjectMutex.unlock();
                }else{

                    Engine::gameObjectMutex.unlock();
                    for(int i=0; i<4; i++){
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
                }*/
            }
            
            createUpdateTextureQueueJob();
            createUpdateDescriptorQueueJob();
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

        for(auto& [id, objptr]:Engine::gameObjectMap){
            delete objptr;
        }

        for (const auto& pair : Engine::threadPool) {
            pair.second->alive=false;
            sem_post(&(Engine::workInQueueSemaphore));
            if (pair.second->thread.joinable()) {
                pair.second->thread.join();
            }
            delete pair.second;
        }

        Engine::threadPool.clear();

        SwapChainManager::cleanupSwapChain(device);

        //for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            //vkDestroyBuffer(device, Engine::uniformBuffers[i], nullptr);
            //vkFreeMemory(device, Engine::uniformBuffersMemory[i], nullptr);
        //}

        vkDestroyDescriptorSetLayout(device, Engine::descriptorSetLayout, nullptr);

        vkDestroyDescriptorPool(device, Engine::descriptorPool, nullptr);

        vkDestroyBuffer(device, Engine::indexVertexBuffer, nullptr);
        vkFreeMemory(device, Engine::indexVertexBufferMemory, nullptr);

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

        vkDestroyRenderPass(device, renderPass, nullptr);

        vkDestroyDevice(device, nullptr);

        if (InstanceManager::enableValidationLayers) {
            InstanceManager::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroySurfaceKHR(instance, surface, nullptr);

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(Engine::window);

        glfwTerminate();
    }

    std::vector<char> Engine::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

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

        Engine::commandPoolMutex.lock();

        vkResetCommandBuffer(Engine::commandBuffers[Engine::currentFrame],  0);

        Engine::commandPoolMutex.unlock();

        Engine::canDeleteObjectMutex.lock();
        Engine::gameObjectMutex.lock();
        Engine::meshMutex.lock();

        if(Engine::gameObjectMap.size()!=0){
            Engine::queueMutex.lock();

            if(Engine::threadsAvailable.getValue()>1 && Engine::jobQueue.size()<Engine::threadsAvailable.getValue()){

                createUpdateObjDescrJob();
                Engine::queueMutex.unlock();

            }else{

                Engine::queueMutex.unlock();
                //Engine::meshMutex.lock();
                Engine::updateGameObjects();
                //Engine::meshMutex.unlock();

                if(Engine::meshBatches.size()!=Engine::descriptorSets.size() || Engine::recreateDescriptors){
                    DescriptorManager::recreateDescriptors(this->device);
                    Engine::recreateDescriptors=false;
                }else{
                    sem_post(&Engine::descriptorsReadySemaphore);
                }
            }

            Engine::recreateVertexIndexBuffer=true;

            //std::cout<<"==== FRAME "<<Engine::frameCount<<" ===="<<std::endl;
            if(Engine::recreateVertexIndexBuffer){
                //Engine::meshMutex.lock();
                BufferManager::recreateVerIndBuffer(this->device,this->physicalDevice,this->graphicsQueue);
                //Engine::meshMutex.unlock();
                Engine::recreateVertexIndexBuffer=false;
            }


            sem_wait(&Engine::safeToMakeInstanceBuffer);

            if(Engine::recreateInstanceBuffer){
                //std::cout<<"Preparing to make instance buffer"<<std::endl;

                //Engine::meshMutex.lock();
                BufferManager::recreateInstanceBuffers(this->device,this->physicalDevice);
                //Engine::meshMutex.unlock();

                Engine::recreateInstanceBuffer=false;

            }
            
        }else{

            sem_post(&Engine::descriptorsReadySemaphore);
        }

        Engine::meshMutex.unlock();
        Engine::gameObjectMutex.unlock();
        sem_wait(&Engine::descriptorsReadySemaphore);

        Engine::commandPoolMutex.lock();
        BufferManager::recordCommandBuffer(Engine::commandBuffers[Engine::currentFrame], imageIndex,device,
        physicalDevice);
        Engine::commandPoolMutex.unlock();
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

                Engine::meshBatches[Engine::meshBatches.size()-1].instances.push_back(
                    InstanceInfo(objPtr->animateCircularMotion(0.0f,0.0f,0.0f,5.0f,2.0f,i*0.25f),textureIndices.at(objPtr->texturePath))
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

    void Engine::initThreadPool(uint16_t poolSize){

        sem_init(&(Engine::workInQueueSemaphore),0,0);
        std::cout<<"\nThreads in pool: "<<poolSize<<"\n"<<std::endl;
        for (uint16_t i=0; i<poolSize; i++){
            WorkerThread* wt = new WorkerThread();

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

        //std::cout<<"Sent the job"<<std::endl;
    }
}
