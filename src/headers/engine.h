#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>

namespace Prometheus{
    class Engine{
    public:
        inline static const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        static VkPresentModeKHR presentMode; 
        /*
        -- VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred 
        to the screen right away, which may result in tearing.

        -- VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image
        from the front of the queue when the display is refreshed and the program inserts
        rendered images at the back of the queue. If the queue is full then the program has
        to wait. This is most similar to vertical sync as found in modern games. The moment
        that the display is refreshed is known as "vertical blank".
        
        -- VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if
        the application is late and the queue was empty at the last vertical blank. Instead of
        waiting for the next vertical blank, the image is transferred right away when it finally
        arrives. This may result in visible tearing.
        
        -- VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of
        blocking the application when the queue is full, the images that are already queued are simply
        replaced with the newer ones. This mode can be used to render frames as fast as possible while
        still avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is
        commonly known as "triple buffering", although the existence of three buffers alone does not 
        necessarily mean that the framerate is unlocked.
        */

        //Window variable
        static GLFWwindow* window;

        static std::vector<VkImage> swapChainImages;
        static std::vector<VkImageView> swapChainImageViews;
        static VkExtent2D swapChainExtent;
        static VkFormat swapChainImageFormat;

        static VkPipelineLayout pipelineLayout;
        static VkRenderPass renderPass;
        static VkPipeline graphicsPipeline;

        static std::vector<VkFramebuffer> swapChainFramebuffers;

        static VkCommandPool commandPool;
        static VkCommandBuffer commandBuffer;

        static VkSemaphore imageAvailableSemaphore;
        static VkSemaphore renderFinishedSemaphore;
        static VkFence inFlightFence;

        void run();
        static std::vector<char> readFile(const std::string& filename);

    private:
        //Window variables
        const uint32_t WIDTH = 800;
        const uint32_t HEIGHT = 600;

        //Hardware and debug variables
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; //GPU
        VkDevice device; //Manages the GPU logically

        //Queues
        VkQueue graphicsQueue;
        VkQueue presentQueue;

        VkSurfaceKHR surface;

        VkSwapchainKHR swapChain;

        void initWindow();
        void initVulkan();
        void mainLoop();
        void createSurface();
        void cleanup();
        void createImageViews();
        void drawFrame();
    };
}
