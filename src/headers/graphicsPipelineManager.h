#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

namespace Prometheus{
    class GraphicsPipelineManager{
    public:
        static void createGraphicsPipeline(VkDevice& device);
        static VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice& device);
        static VkPipelineShaderStageCreateInfo createShaderStageInfo(VkStructureType sType,
        VkShaderStageFlagBits stage,
        VkShaderModule& module,
        const char* pName,
        const VkSpecializationInfo* pSpecializationInfo
        );
    };
}