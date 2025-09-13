#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

namespace Prometheus{
    class Engine{
    public:
        void run();

    private:
        //Window variables
        GLFWwindow* window;
        const uint32_t WIDTH = 800;
        const uint32_t HEIGHT = 600;

        //Hardware and debug variables
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; //GPU
        VkDevice device; //Manages the GPU logically

        VkQueue graphicsQueue;

        void initWindow();
        void initVulkan();
        void mainLoop();
        void cleanup();
    };
}
