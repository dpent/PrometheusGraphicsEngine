#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace Prometheus{
    class ComputePipelineManager{
        public:

        static void createComputePipeline(VkDevice& device);
    };
}