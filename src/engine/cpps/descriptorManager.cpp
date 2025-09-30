#include "../headers/descriptorManager.h"
#include "../headers/engine.h"


using namespace Prometheus;

namespace Prometheus{
    void DescriptorManager::createDescriptorSetLayout(VkDevice& device){
        /*VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; //This will be used in the vertex shader
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //This as well (We also need to specify in which shader stages the descriptor is going to be referenced)
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.pImmutableSamplers = nullptr;*/

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 0;
        samplerLayoutBinding.descriptorCount = 64; // Max per batch
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 1> bindings = {samplerLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &Engine::descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void DescriptorManager::createDescriptorPool(VkDevice& device){
        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = static_cast<uint32_t>(Engine::meshBatches.size() * 64);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(Engine::meshBatches.size());

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &Engine::descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void DescriptorManager::createDescriptorSets(VkDevice& device){

        Engine::descriptorSets.resize(Engine::meshBatches.size());
        uint32_t i=0;
        for (auto &batch : Engine::meshBatches) {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = Engine::descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &Engine::descriptorSetLayout;

            if (vkAllocateDescriptorSets(device, &allocInfo, &Engine::descriptorSets[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor set!");
            }

            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.reserve(batch.textures.size());

            for (auto tex : batch.textures) {
                VkDescriptorImageInfo info{};
                info.sampler     = tex.textureSampler;
                info.imageView   = tex.textureImageView;
                info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos.push_back(info);
            }

            VkWriteDescriptorSet write{};
            write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet          = Engine::descriptorSets[i];
            write.dstBinding      = 0;
            write.dstArrayElement = 0;
            write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            write.descriptorCount = static_cast<uint32_t>(imageInfos.size());
            write.pImageInfo      = imageInfos.data();

            vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

            i++;
        }
    }

    void DescriptorManager::recreateDescriptors(VkDevice& device){

        vkDeviceWaitIdle(device);

        if(Engine::descriptorPool != VK_NULL_HANDLE){
            vkDestroyDescriptorPool(device, Engine::descriptorPool, nullptr);
        }

        DescriptorManager::createDescriptorPool(device);
        DescriptorManager::createDescriptorSets(device);
    }
}