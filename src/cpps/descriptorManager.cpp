#include "../headers/descriptorManager.h"
#include "../headers/engine.h"


using namespace Prometheus;

namespace Prometheus{
    void DescriptorManager::createDescriptorSetLayout(){
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; //This will be used in the vertex shader
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //This as well (We also need to specify in which shader stages the descriptor is going to be referenced)
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.pImmutableSamplers = nullptr;
    }
}