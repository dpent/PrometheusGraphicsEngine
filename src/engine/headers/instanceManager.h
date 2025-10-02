#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cstring>

namespace Prometheus{
    class InstanceManager{
    
    private:
    public:
        inline static const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"
        };

        #ifdef NDEBUG
            static const bool enableValidationLayers = false;
        #else
            static const bool enableValidationLayers = true;
        #endif

        static void createInstance(VkInstance& instance);
        static bool checkValidationLayerSupport();
        static std::vector<const char*> getRequiredExtensions();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData
        );
        static VkResult CreateDebugUtilsMessengerEXT(
            VkInstance instance, 
            const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
            const VkAllocationCallbacks* pAllocator, 
            VkDebugUtilsMessengerEXT* pDebugMessenger
        );
        static void DestroyDebugUtilsMessengerEXT(
            VkInstance instance, 
            VkDebugUtilsMessengerEXT debugMessenger, 
            const VkAllocationCallbacks* pAllocator
        );
        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        static void setupDebugMessenger(VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger);
    };
}