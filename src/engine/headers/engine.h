#pragma once

#include <vulkan/vulkan_core.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp>

//#define STB_IMAGE_IMPLEMENTATION //Don't redo this in another file. Simply include the .h file
#include "stb_image.h"

#include <chrono>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>
#include <array>
#include <glm/glm.hpp>
#include <mutex>
#include <queue>
#include <thread>
#include "../../objects/headers/gameObject.h"
#include "../../objects/headers/mesh.h"
#include "../../threads/headers/job.h"
#include "../../threads/headers/workerThread.h"
#include <list>
#include "../../threads/headers/threadSafeNumber.h"

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
        static VkSwapchainKHR swapChain;

        static VkDescriptorSetLayout descriptorSetLayout;
        static VkPipelineLayout pipelineLayout;
        static VkRenderPass renderPass;
        static VkPipeline graphicsPipeline;

        static std::vector<VkFramebuffer> swapChainFramebuffers;

        static VkCommandPool commandPool;
        static std::vector<VkCommandBuffer> commandBuffers;

        static std::vector<VkSemaphore> imageAvailableSemaphores;
        static std::vector<VkSemaphore> renderFinishedSemaphores;
        static std::vector<VkFence> inFlightFences;
        static sem_t descriptorsReadySemaphore;
        static sem_t safeToMakeInstanceBuffer;
        static sem_t verIndBufferComplete;
        static sem_t instanceBufferReady;
        static std::mutex gameObjectMutex;
        static std::mutex canDeleteObjectMutex;
        static std::mutex textureMutex;
        static std::mutex textureQueuedMutex;
        static std::mutex graphicsQueueMutex;
        static std::mutex commandPoolMutex;
        static std::mutex meshMutex;
        static std::mutex descriptorQueuedMutex;

        static const int MAX_FRAMES_IN_FLIGHT = 2;
        static uint32_t currentFrame;

        static bool framebufferResized;

        static std::vector<Vertex> vertices;
        static std::vector<uint32_t> indices;

        static VkBuffer indexVertexBuffer;
        static VkDeviceMemory indexVertexBufferMemory;
        static uint64_t indexVertexBufferSize;

        static std::vector<VkBuffer> instanceBuffers;
        static std::vector<VkDeviceMemory> instanceBufferMemories;
        static std::vector<void*> instanceBuffersMapped;
        static uint64_t instanceBufferSize;

        static VkDeviceSize indexOffset;

        static std::vector<VkBuffer> uniformBuffers;
        static std::vector<VkDeviceMemory> uniformBuffersMemory;
        static std::vector<void*> uniformBuffersMapped;

        static VkDescriptorPool descriptorPool;
        static std::vector<VkDescriptorSet> descriptorSets;
        static std::list<VkDescriptorPool> descriptorDeleteQueue;
        static std::list<int> framesSinceDescriptorQueuedForDeletion;

        static glm::mat4 model;
        static glm::mat4 view;
        static glm::mat4 proj; //Camera movement is based on these

        static std::unordered_map<uint64_t,GameObject*> gameObjectMap;

        static VkPhysicalDeviceProperties physicalDeviceProperties;
        static VkPhysicalDeviceFeatures physicalDeviceFeatures;
        static std::unordered_map<std::string, Texture> textureMap;
        static std::unordered_map<std::string,std::vector<uint64_t>> objectIdsByTexture;
        static std::unordered_map<std::string, std::vector<Texture>> texturesQueuedForDeletion;
        static std::unordered_map<std::string, std::vector<int>> framesSinceTextureQueuedForDeletion;

        static std::unordered_map<std::string,Mesh> meshMap;
        static std::unordered_map<std::string,bool> meshesLoading;
        static std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>> objectsByMesh;
        static std::vector<MeshBatch> meshBatches;

        static VkImage depthImage;
        static VkDeviceMemory depthImageMemory;
        static VkImageView depthImageView;

        static bool recreateVertexIndexBuffer;
        static bool recreateInstanceBuffer;
        static bool recreateInstBuffer;
        static bool recreateDescriptors;

        static VkSampleCountFlagBits msaaSamples; //MULTISAMPLING VARIABLES
        static VkImage colorImage;
        static VkDeviceMemory colorImageMemory;
        static VkImageView colorImageView;

        static std::unordered_map<std::thread::id, WorkerThread*> threadPool;
        static std::queue<Job> jobQueue;
        static std::queue<Job> deferredJobQueue;
        static std::mutex queueMutex;
        static sem_t workInQueueSemaphore;
        static uint64_t frameCount;

        static SafeUint16_t threadsAvailable;

        void run();
        static std::vector<char> readFile(const std::string& filename);
        static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
        static void updateGameObjects();
        static VkSampleCountFlagBits getMaxUsableSampleCount(VkPhysicalDevice& physicalDevice);
        static void initThreadPool(uint16_t poolSize);
        static std::vector<std::queue<Job*>> batchJobs();
        static void createInstanceBufferUpdateJob();

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

        void initWindow();
        void initVulkan();
        void mainLoop();
        void createSurface();
        void cleanup();
        void drawFrame();
        void createUpdateTextureQueueJob();
        void updateMeshDataStructures();
        void createUpdateDescriptorQueueJob();
        void createUpdateObjDescrJob();
        void createVertexIndexBufferUpdateJob();
        void createInstanceBufferRemakeJob();
    };
}
