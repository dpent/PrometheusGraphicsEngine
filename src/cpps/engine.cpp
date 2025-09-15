#include "../headers/engine.h"
#include "../headers/deviceManager.h"
#include "../headers/swapChainManager.h"
#include "../headers/graphicsPipelineManager.h"
#include "../headers/renderPassManager.h"
#include "../headers/bufferManager.h"
#include "../headers/syncManager.h"
#include <vulkan/vulkan_core.h>


using namespace Prometheus;

//DEFINE STATIC VARIABLES BEFORE VULKAN INIT
GLFWwindow* Engine::window = nullptr;
VkPresentModeKHR Engine::presentMode=VK_PRESENT_MODE_MAILBOX_KHR;

std::vector<VkImage> Engine::swapChainImages;
std::vector<VkImageView> Engine::swapChainImageViews;
VkExtent2D Engine::swapChainExtent;
VkFormat Engine::swapChainImageFormat;
VkSwapchainKHR Engine::swapChain;

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

        InstanceManager::createInstance(this->instance);
        InstanceManager::setupDebugMessenger(this->instance,this->debugMessenger);

        Engine::createSurface();

        DeviceManager::pickPhysicalDevice(this->instance,this->physicalDevice, this->surface);
        DeviceManager::createLogicalDevice(this->physicalDevice, this->device, this->graphicsQueue,this->presentQueue, this->surface);

        SwapChainManager::createSwapChain(this->surface,this->physicalDevice,this->device, Engine::swapChain);
        SwapChainManager::createImageViews(this->device);

        RenderPassManager::createRenderPass(this->device);

        GraphicsPipelineManager::createGraphicsPipeline(this->device);

        BufferManager::createFrameBuffers(this->device);
        BufferManager::createCommandPool(this->physicalDevice, this->surface,this->device);
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

        SwapChainManager::cleanupSwapChain(device);

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

    void Engine::createImageViews(){
        Engine::swapChainImageViews.resize(Engine::swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = Engine::swapChainImageFormat;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }

        }
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
        vkWaitForFences(device, 1, &Engine::inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(device, Engine::swapChain, UINT64_MAX,
            Engine::imageAvailableSemaphores[currentFrame], 
            VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            SwapChainManager::recreateSwapChain(surface,physicalDevice,device,presentQueue);
            framebufferResized = false;
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { //Lacks logic for suboptimal swap chains
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(device, 1, &Engine::inFlightFences[currentFrame]);

        vkResetCommandBuffer(Engine::commandBuffers[currentFrame],  0);
        BufferManager::recordCommandBuffer(Engine::commandBuffers[currentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {Engine::imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &Engine::commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {Engine::renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, Engine::inFlightFences[currentFrame]) != VK_SUCCESS) {
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

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void Engine::frameBufferResizeCallback(GLFWwindow* window, int width, int height){
        auto app = reinterpret_cast<Engine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
}
