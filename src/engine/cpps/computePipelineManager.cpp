#include "../headers/computePipelineManager.h"
#include "../headers/engine.h"
#include "../headers/graphicsPipelineManager.h"

using namespace Prometheus;

namespace Prometheus{

    void ComputePipelineManager::createComputePipeline(VkDevice& device){
        auto computeShaderCode = Engine::readFile("../build/particlesComp.spv");

        VkShaderModule computeShaderModule = GraphicsPipelineManager::createShaderModule(computeShaderCode, device);
        
        VkPipelineShaderStageCreateInfo shaderStage = 
            GraphicsPipelineManager::createShaderStageInfo(
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                VK_SHADER_STAGE_COMPUTE_BIT,
                computeShaderModule,
                "main",
                nullptr
            );
        
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(float);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &Engine::computeSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &Engine::computePipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = Engine::computePipelineLayout;    
        pipelineInfo.stage = shaderStage;

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Engine::computePipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline!");
        }

        vkDestroyShaderModule(device, computeShaderModule, nullptr);

    }
}