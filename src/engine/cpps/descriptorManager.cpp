#include "../headers/descriptorManager.h"
#include "../headers/engine.h"


using namespace Prometheus;

namespace Prometheus{
    void DescriptorManager::createDescriptorSetLayout(VkDevice& device){
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0; //This will be used in the vertex shader
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //This as well (We also need to specify in which shader stages the descriptor is going to be referenced)
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 64; // Max per batch
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding,samplerLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &Engine::descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void DescriptorManager::createDescriptorPool(VkDevice& device){
        std::array<VkDescriptorPoolSize, 2> poolSizes{};

        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(Engine::meshBatches.size() * Engine::MAX_FRAMES_IN_FLIGHT);

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(Engine::meshBatches.size() * 64); // max 64 textures per batch

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
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

            std::vector<VkDescriptorBufferInfo> bufferInfos{};
            bufferInfos.reserve(Engine::MAX_FRAMES_IN_FLIGHT);

            for(size_t j=0; j<Engine::MAX_FRAMES_IN_FLIGHT; j++){

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = Engine::uniformBuffers[j]; // VkBuffer for UBO
                bufferInfo.offset = 0;
                bufferInfo.range  = sizeof(UBOData);

                bufferInfos.push_back(bufferInfo);
            }

            VkWriteDescriptorSet uboWrite{};
            uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uboWrite.dstSet = Engine::descriptorSets[i];
            uboWrite.dstBinding = 0; // binding 0 for UBO
            uboWrite.dstArrayElement = 0;
            uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboWrite.descriptorCount = 1;
            uboWrite.pBufferInfo = bufferInfos.data();

            std::vector<VkDescriptorImageInfo> imageInfos;
            imageInfos.reserve(batch->textures.size());

            for (auto tex : batch->textures) {
                VkDescriptorImageInfo info{};
                info.sampler     = (*tex).textureSampler;
                info.imageView   = (*tex).textureImageView;
                info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfos.push_back(info);
            }

            VkWriteDescriptorSet textureWrite{};
            textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            textureWrite.dstSet = Engine::descriptorSets[i];
            textureWrite.dstBinding = 1; // binding 1 for textures
            textureWrite.dstArrayElement = 0;
            textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
            textureWrite.pImageInfo = imageInfos.data();

            std::array<VkWriteDescriptorSet, 2> writes = { uboWrite, textureWrite };
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

            i++;
        }

    }

    void DescriptorManager::recreateDescriptors(VkDevice& device){

        Job j = Job(RECREATE_DESCRIPTORS);
        j.data.emplace_back(std::in_place_type<VkDevice*>, &device);
        j.data.emplace_back(std::in_place_type<sem_t*>,&Engine::descriptorsReadySemaphore);

        Engine::jobQueue.push(j);

        sem_post(&Engine::workInQueueSemaphore);

    }
}