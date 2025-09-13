#include "../headers/engine.h"
#include "../headers/deviceManager.h"

using namespace Prometheus;

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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    }

    void Engine::initVulkan() {

        window = glfwCreateWindow(WIDTH, HEIGHT, "Prometheus", nullptr, nullptr);

        InstanceManager::createInstance(this->instance);
        InstanceManager::setupDebugMessenger(this->instance,this->debugMessenger);

        DeviceManager::pickPhysicalDevice(this->instance,this->physicalDevice);
        DeviceManager::createLogicalDevice(this->physicalDevice, this->device, this->graphicsQueue);
    }

    void Engine::mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void Engine::cleanup() {
        vkDestroyDevice(device, nullptr);

        if (InstanceManager::enableValidationLayers) {
            InstanceManager::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }
}
