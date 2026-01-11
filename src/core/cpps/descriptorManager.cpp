#include "../headers/descriptorManager.h"
#include "../headers/engine.h"


void DescriptorManager::createGraphicsDescriptorSetLayout() {

    if (Engine::graphicsDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::graphicsDescriptor.layout, nullptr);
    }
    
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding instanceDataSSBO{};
    instanceDataSSBO.binding = 1;
    instanceDataSSBO.descriptorCount = 1;
    instanceDataSSBO.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceDataSSBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 2;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding shadowMapBinding{};
    shadowMapBinding.binding = 3;
    shadowMapBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    shadowMapBinding.descriptorCount = 128;

    VkDescriptorSetLayoutBinding texturesBinding{}; //Textures
    texturesBinding.binding = 4;
    texturesBinding.descriptorCount = Engine::MAX_TEXTURES;
    texturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texturesBinding.pImmutableSamplers = nullptr;
    texturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = {    
    samplerBinding,
    instanceDataSSBO,
    uboLayoutBinding,
    shadowMapBinding,
    texturesBinding
    };

    std::array<VkDescriptorBindingFlags, 5> bindingFlags = {
    0, // samplerBinding (binding 0) no flags
    0, // instanceDataSSBO (binding 1) no flags
    0, // uboLayoutBinding (binding 2) no flags
    0, // shadowMapBinding (binding 3)
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT // texturesBinding (binding 4)
    };

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{};
    flagsInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
    flagsInfo.pBindingFlags = bindingFlags.data();

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
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Engine::graphicsDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 5> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 1;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = 1;

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[3].descriptorCount = 128;

    poolSizes[4].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[4].descriptorCount = Engine::MAX_TEXTURES;

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
    std::vector<VkDescriptorImageInfo> shadowMaps;

    Engine::graphicsDescriptor.sets.resize(1);

    uint32_t variableDescriptorCount = static_cast<uint32_t>(Engine::textures.size);

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

    uint32_t count = 0;
    Texture* texture = Engine::textures.head;
    while(texture != nullptr) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = texture->image.view;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        textures.push_back(imageInfo);
        texture->index = count;
        count++;
        texture = texture->next;
    }

    for (size_t i = 0; i < Engine::shadowCreatingLights.size; i++) {
        VkDescriptorImageInfo shadowInfo{};
        shadowInfo.imageView = Engine::shadowMaps.views[i];
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        shadowMaps.push_back(shadowInfo);
    }

    for (size_t i = Engine::shadowCreatingLights.size; i < 128; i++) {
        VkDescriptorImageInfo shadowInfo{};
        shadowInfo.imageView = Engine::shadowMaps.views[0];
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        shadowMaps.push_back(shadowInfo);
    }

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = Engine::linearSampler;

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = Engine::instanceDataSSBO.buffer; // your Buffer struct
    bufferInfo.offset = 0;
    bufferInfo.range = Engine::instanceDataSSBO.size;

    VkDescriptorBufferInfo uboInfo{};
    uboInfo.buffer = Engine::uniformLightBuffer.buffer; // VkBuffer for UBO
    uboInfo.offset = 0;
    uboInfo.range = sizeof(LightUBOData);

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = Engine::graphicsDescriptor.sets[0];
    write.dstBinding = 4;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.descriptorCount = static_cast<uint32_t>(Engine::textures.size);
    write.pImageInfo = textures.data();

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstSet = Engine::graphicsDescriptor.sets[0];
    samplerWrite.dstBinding = 0;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerWrite.descriptorCount = 1;
    samplerWrite.pImageInfo = &samplerInfo;

    VkWriteDescriptorSet instanceSSBOWrite{};
    instanceSSBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    instanceSSBOWrite.dstSet = Engine::graphicsDescriptor.sets[0];
    instanceSSBOWrite.dstBinding = 1;
    instanceSSBOWrite.dstArrayElement = 0;
    instanceSSBOWrite.descriptorCount = 1;
    instanceSSBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceSSBOWrite.pBufferInfo = &bufferInfo;

    VkWriteDescriptorSet uboWrite{};
    uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uboWrite.dstSet = Engine::graphicsDescriptor.sets[0];
    uboWrite.dstBinding = 2;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &uboInfo;

    VkWriteDescriptorSet shadowWrite{};
    shadowWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    shadowWrite.dstSet = Engine::graphicsDescriptor.sets[0];
    shadowWrite.dstBinding = 3;
    shadowWrite.dstArrayElement = 0;
    shadowWrite.descriptorCount = static_cast<uint32_t>(Engine::shadowCreatingLights.size);
    shadowWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    shadowWrite.pImageInfo = shadowMaps.data();

    std::array<VkWriteDescriptorSet, 5> writes = {
        write,
        samplerWrite,
        instanceSSBOWrite,
        uboWrite,
        shadowWrite
    };

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}