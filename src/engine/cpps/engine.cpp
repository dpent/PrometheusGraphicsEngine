#include "../headers/engine.h"
#include "../headers/deviceManager.h"
#include "../headers/swapChainManager.h"
#include "../headers/graphicsPipelineManager.h"
#include "../headers/renderPassManager.h"
#include "../headers/bufferManager.h"
#include "../headers/syncManager.h"
#include <vulkan/vulkan_core.h>
#include "../headers/descriptorManager.h"

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

uint32_t Engine::currentFrame = 0;

bool Engine::framebufferResized = false;

std::vector<Vertex> Engine::vertices;
std::vector<uint32_t> Engine::indices;

VkBuffer Engine::indexVertexBuffer= nullptr;
VkDeviceMemory Engine::indexVertexBufferMemory= nullptr;

std::vector<VkBuffer> Engine::instanceBuffers;
std::vector<VkDeviceMemory> Engine::instanceBufferMemories;
std::vector<void*> Engine::instanceBuffersMapped;

VkDeviceSize Engine::indexOffset=0;

std::vector<VkBuffer> Engine::uniformBuffers;
std::vector<VkDeviceMemory> Engine::uniformBuffersMemory;
std::vector<void*> Engine::uniformBuffersMapped;

VkDescriptorPool Engine::descriptorPool;
std::vector<VkDescriptorSet> Engine::descriptorSets;

glm::mat4 Engine::model;
glm::mat4 Engine::view;
glm::mat4 Engine::proj;

std::vector<GameObject*> Engine::gameObjects;
std::unordered_map<uint64_t,GameObject*> Engine::gameObjectMap;

VkPhysicalDeviceProperties Engine::physicalDeviceProperties;
VkPhysicalDeviceFeatures Engine::physicalDeviceFeatures;

std::unordered_map<std::string, Texture> Engine::textureMap;
std::unordered_map<std::string,std::vector<uint64_t>> Engine::objectIdsByTexture;

std::unordered_map<std::string,Mesh> Engine::meshMap;
std::vector<InstanceInfo> Engine::instances;
std::map<std::string,std::map<uint64_t,GameObject*>> Engine::objectsByMesh;
std::vector<MeshBatch> Engine::meshBatches;

VkImage Engine::depthImage;
VkDeviceMemory Engine::depthImageMemory;
VkImageView Engine::depthImageView;

bool Engine::recreateVertexIndexInstanceBuffer=true;
bool Engine::recreateDescriptors=true;

VkSampleCountFlagBits Engine::msaaSamples=VK_SAMPLE_COUNT_1_BIT;
VkImage Engine::colorImage;
VkDeviceMemory Engine::colorImageMemory;
VkImageView Engine::colorImageView;

ThreadManager Engine::threadManager = ThreadManager();

//uint32_t Engine::frameCounter=0;

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

        threadManager.start(12);
        threadManager.detach();

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

        new GameObject("../textures/statue.jpg","../models/stanford_sphere.obj",STBI_rgb_alpha,
        device,physicalDevice,graphicsQueue);

        new GameObject("../textures/angel.jpg","../models/cube.obj",STBI_rgb_alpha,
        device,physicalDevice,graphicsQueue);

        new GameObject("../textures/viking_room.png","../models/viking_room.obj",STBI_rgb_alpha,
        device,physicalDevice,graphicsQueue);

        //BufferManager::createUniformBuffers(this->device,this->physicalDevice);

        Engine::instanceBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBufferMemories.resize(MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBuffersMapped.resize(Engine::MAX_FRAMES_IN_FLIGHT);

        BufferManager::createCommandBuffers(this->device);

        SyncManager::createSyncObjects(this->device);
    }

    void Engine::mainLoop() {
        while (!glfwWindowShouldClose(Engine::window)) {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(device);
    }

    void Engine::createSurface(){
        if (glfwCreateWindowSurface(instance, Engine::window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void Engine::cleanup() {

        for(size_t i=0; i<Engine::gameObjects.size(); i++){
            Engine::gameObjects[i]->terminate(device);
            delete Engine::gameObjects[i];
        }

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

        vkResetCommandBuffer(Engine::commandBuffers[Engine::currentFrame],  0);
        Engine::meshBatches.clear();

        if(Engine::gameObjects.size()!=0){
            if(Engine::recreateVertexIndexInstanceBuffer){
                BufferManager::recreateVerIndBuffer(this->device,this->physicalDevice,this->graphicsQueue);

                Engine::updateGameObjects();
                
                BufferManager::recreateInstanceBuffers(this->device,this->physicalDevice,this->graphicsQueue);

                Engine::recreateVertexIndexInstanceBuffer=false;
            }else{
                Engine::updateGameObjects();
            }

            if(Engine::recreateDescriptors){
                DescriptorManager::recreateDescriptors(this->device);

                Engine::recreateDescriptors=false;
            }
        }

        BufferManager::recordCommandBuffer(Engine::commandBuffers[Engine::currentFrame], imageIndex,device,
        physicalDevice,graphicsQueue);

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

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, Engine::inFlightFences[Engine::currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {Engine::swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || Engine::framebufferResized) {
            SwapChainManager::recreateSwapChain(surface,physicalDevice,device,presentQueue);
            framebufferResized = false;
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        Engine::currentFrame = (Engine::currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Engine::frameBufferResizeCallback(GLFWwindow* window, int width, int height){
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    void Engine::updateGameObjects(){
        int i=0;
        for (auto& [meshName, innerMap] : Engine::objectsByMesh) {

            uint64_t currIndex=0;
            std::unordered_map<std::string,uint64_t> textureIndices;
            Engine::meshBatches.push_back(MeshBatch(meshName));

            for (auto& [id, objPtr] : innerMap) {

                if (textureIndices.find(objPtr->texturePath) == textureIndices.end()) {
                    Engine::meshBatches[Engine::meshBatches.size()-1].textures.push_back(Engine::textureMap[objPtr->texturePath]);
                    textureIndices[objPtr->texturePath] = currIndex;
                    currIndex++; 
                }

                Engine::meshBatches[Engine::meshBatches.size()-1].instances.push_back(
                    InstanceInfo(Engine::gameObjects[i]->animateCircularMotion(0.0f,0.0f,0.0f,5.0f,2.0f,i*0.25f),textureIndices.at(objPtr->texturePath))
                );
                Engine::meshBatches[Engine::meshBatches.size()-1].ids.push_back(objPtr->id);
                i++;
            }
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
}
