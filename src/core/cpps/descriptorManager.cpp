#include "../headers/descriptorManager.h"
#include "../headers/engine.h"


void DescriptorManager::createGraphicsDescriptorSetLayout() {

    if (Engine::graphicsDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::graphicsDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding texturesBinding{}; //Textures
    texturesBinding.binding = 0;
    texturesBinding.descriptorCount = Engine::MAX_TEXTURES;
    texturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texturesBinding.pImmutableSamplers = nullptr;
    texturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
    texturesBinding
    };

    VkDescriptorBindingFlags flags =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = 1;
    flagsInfo.pBindingFlags = &flags;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();
    layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    layoutInfo.pNext = &flagsInfo;

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Engine::graphicsDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorManager::createGraphicsDescriptorPool() {

    if (Engine::graphicsDescriptor.pool != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(Engine::deviceInfo.logicalDevice, Engine::graphicsDescriptor.pool, nullptr);
    }

    std::array<VkDescriptorPoolSize, 1> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[0].descriptorCount = Engine::MAX_TEXTURES;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1; //NOT PER FRAME. ITS 1 GLOBAL
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Engine::graphicsDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorManager::createGraphicsDescriptorSets() {

    std::vector<VkDescriptorImageInfo> textures;

    Engine::graphicsDescriptor.sets.resize(1);

    uint32_t variableDescriptorCount = static_cast<uint32_t>(Engine::textures.size());

    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    variableCountInfo.descriptorSetCount = 1;
    variableCountInfo.pDescriptorCounts = &variableDescriptorCount;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &variableCountInfo;
    allocInfo.descriptorPool = Engine::graphicsDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &Engine::graphicsDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, &Engine::graphicsDescriptor.sets[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    for (auto texture : Engine::textures) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = texture->view;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        textures.push_back(imageInfo);
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = Engine::graphicsDescriptor.sets[0];
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.descriptorCount = static_cast<uint32_t>(Engine::textures.size());
    write.pImageInfo = textures.data();

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, 1, &write, 0, nullptr);
}