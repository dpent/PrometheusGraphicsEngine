#include "../headers/descriptorManager.h"
#include "../headers/engine.h"
#include "../../physics/headers/particleEffect.h"

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

    VkDescriptorSetLayoutBinding texturesBinding{}; //Textures
    texturesBinding.binding = 5;
    texturesBinding.descriptorCount = Engine::MAX_TEXTURES;
    texturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texturesBinding.pImmutableSamplers = nullptr;
    texturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {    
    samplerBinding,
    instanceDataSSBO,
    uboLayoutBinding,
    texturesBinding
    };

    std::array<VkDescriptorBindingFlags, 4> bindingFlags = {
    0, // samplerBinding (binding 0) no flags
    0, // instanceDataSSBO (binding 1) no flags
    0, // uboLayoutBinding (binding 2) no flags
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT // texturesBinding (binding 5)
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

    std::array<VkDescriptorPoolSize, 4> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 1;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[2].descriptorCount = 1;

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[3].descriptorCount = Engine::MAX_TEXTURES;

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

    uint32_t count = 0;
    Texture* texture = Engine::textures.head;
    while (texture != nullptr) {
        count++;

        if (texture->hasNormals) {
            count++;
        }

        texture = texture->next;
    }

    uint32_t variableDescriptorCount = static_cast<uint32_t>(count);

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

    count = 0;
    texture = Engine::textures.head;
    while(texture != nullptr) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = texture->image.view;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        textures.push_back(imageInfo);
        texture->index = count;
        count++;
        
        if (texture->hasNormals) {
            VkDescriptorImageInfo normalMapInfo{};
            normalMapInfo.imageView = texture->normalMap.view;
            normalMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            textures.push_back(normalMapInfo);
            count++;
        }

        texture = texture->next;
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
    write.dstBinding = 5;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.descriptorCount = static_cast<uint32_t>(textures.size());
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

    std::array<VkWriteDescriptorSet, 4> writes = {
        write,
        samplerWrite,
        instanceSSBOWrite,
        uboWrite
    };

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void DescriptorManager::createShadowLightsSetLayout() {
    if (Engine::shadowLightsDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::shadowLightsDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding uboShadowLightLayoutBinding{};
    uboShadowLightLayoutBinding.binding = 3;
    uboShadowLightLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    uboShadowLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboShadowLightLayoutBinding.descriptorCount = 1;
    uboShadowLightLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding shadowMapBinding{};
    shadowMapBinding.binding = 4;
    shadowMapBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
    shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    shadowMapBinding.descriptorCount = 64;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
    uboShadowLightLayoutBinding,
    shadowMapBinding
    };

    std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
    0, // uboShadowLightLayoutBinding (binding 3) no flags
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT, // shadowMapBinding (binding 4) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Engine::shadowLightsDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorManager::createShadowLightsPool() {

    if (Engine::shadowLightsDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Engine::shadowLightsDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[1].descriptorCount = 64;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1; //NOT PER FRAME. ITS 1 GLOBAL
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Engine::shadowLightsDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorManager::createShadowLightsSets() {

    Engine::shadowLightsDescriptor.sets.resize(1);
    std::vector<VkDescriptorImageInfo> shadowMaps;

    uint32_t variableDescriptorCount = static_cast<uint32_t>(Engine::shadowCreatingLights.size) == 0 ? 1 : static_cast<uint32_t>(Engine::shadowCreatingLights.size);
    VkDescriptorSetVariableDescriptorCountAllocateInfo variableCountInfo{};
    variableCountInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    variableCountInfo.descriptorSetCount = 1;
    variableCountInfo.pDescriptorCounts = &variableDescriptorCount;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &variableCountInfo;
    allocInfo.descriptorPool = Engine::shadowLightsDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &Engine::shadowLightsDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, &Engine::shadowLightsDescriptor.sets[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorBufferInfo shadowUboInfo{};
    shadowUboInfo.buffer = Engine::uniformShadowLightBuffer.buffer; // VkBuffer for UBO
    shadowUboInfo.offset = 0;
    shadowUboInfo.range = sizeof(ShadowLightUBOData);

    if (Engine::shadowCreatingLights.size == 0) {
        VkDescriptorImageInfo shadowInfo{};
        shadowInfo.imageView = Engine::dummyImage->view;
        shadowInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        shadowMaps.push_back(shadowInfo);
    }
    else {

        for (size_t i = 0; i < Engine::shadowCreatingLights.size; i++) {
            VkDescriptorImageInfo shadowInfo{};
            shadowInfo.imageView = Engine::shadowMaps->views[i];
            shadowInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

            shadowMaps.push_back(shadowInfo);
        }

    }

    VkWriteDescriptorSet shadowUboWrite{};
    shadowUboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    shadowUboWrite.dstSet = Engine::shadowLightsDescriptor.sets[0];
    shadowUboWrite.dstBinding = 3;
    shadowUboWrite.dstArrayElement = 0;
    shadowUboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    shadowUboWrite.descriptorCount = 1;
    shadowUboWrite.pBufferInfo = &shadowUboInfo;

    VkWriteDescriptorSet shadowWrite{};
    shadowWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    shadowWrite.dstSet = Engine::shadowLightsDescriptor.sets[0];
    shadowWrite.dstBinding = 4;
    shadowWrite.dstArrayElement = 0;
    shadowWrite.descriptorCount = static_cast<uint32_t>(Engine::shadowCreatingLights.size) == 0 ? 1 : static_cast<uint32_t>(Engine::shadowCreatingLights.size);
    shadowWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    shadowWrite.pImageInfo = shadowMaps.data();

    std::array<VkWriteDescriptorSet, 2> writes = {
        shadowUboWrite,
        shadowWrite
    };
    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);

}

void DescriptorManager::createDebugSetLayout() {

    if (Debug::debugDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Debug::debugDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding instanceDataSSBO{};
    instanceDataSSBO.binding = 0;
    instanceDataSSBO.descriptorCount = 1;
    instanceDataSSBO.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceDataSSBO.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
    instanceDataSSBO,
    };

    std::array<VkDescriptorBindingFlags, 1> bindingFlags = {
    0, // instanceDataSSBO (binding 0) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Debug::debugDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

}

void DescriptorManager::createDebugDescriptorPool() {

    if (Debug::debugDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Debug::debugDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 1> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1; //NOT PER FRAME. ITS 1 GLOBAL
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Debug::debugDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }

}

void DescriptorManager::createDebugDescriptorSets() {

    Debug::debugDescriptor.sets.resize(1);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Debug::debugDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &Debug::debugDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, &Debug::debugDescriptor.sets[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = Debug::lineSSBO.buffer; // your Buffer struct
    bufferInfo.offset = 0;
    bufferInfo.range = Debug::lineSSBO.size;

    VkWriteDescriptorSet instanceSSBOWrite{};
    instanceSSBOWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    instanceSSBOWrite.dstSet = Debug::debugDescriptor.sets[0];
    instanceSSBOWrite.dstBinding = 0;
    instanceSSBOWrite.dstArrayElement = 0;
    instanceSSBOWrite.descriptorCount = 1;
    instanceSSBOWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    instanceSSBOWrite.pBufferInfo = &bufferInfo;

    std::array<VkWriteDescriptorSet, 1> writes = {
        instanceSSBOWrite
    };

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void DescriptorManager::createParticleSetLayout() {
    if (Engine::particleDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::particleDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding SSBO1{};
    SSBO1.binding = 0;
    SSBO1.descriptorCount = 1;
    SSBO1.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    SSBO1.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding SSBO2{};
    SSBO2.binding = 1;
    SSBO2.descriptorCount = 1;
    SSBO2.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    SSBO2.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
    SSBO1,
    SSBO2
    };

    std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
    0, // SSBO1 (binding 0) no flags
	0, // SSBO2 (binding 1) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Engine::particleDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

}

void DescriptorManager::createParticleDescriptorPool() {

    if (Engine::particleDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Engine::particleDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = Engine::MAX_FRAMES_IN_FLIGHT * static_cast<uint32_t>(Engine::particleEffects.size + 1); //For some reason this works

	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = Engine::MAX_FRAMES_IN_FLIGHT * static_cast<uint32_t>(Engine::particleEffects.size + 1);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = Engine::MAX_FRAMES_IN_FLIGHT * static_cast<uint32_t>(Engine::particleEffects.size + 1); //PER FRAME
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Engine::particleDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

#ifdef RAY_TRACING
void DescriptorManager::createRayTracingSetLayout() {
    if (Engine::rayTracingDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::rayTracingDescriptor.layout, nullptr);
        Engine::layoutsUsed.clear();
    }

    VkDescriptorSetLayoutBinding storageImage{};
    storageImage.binding = 0;
    storageImage.descriptorCount = 1;
    storageImage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    storageImage.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding accumulationImage{};
    accumulationImage.binding = 1;
    accumulationImage.descriptorCount = 1;
    accumulationImage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    accumulationImage.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding accumulationImage2{};
    accumulationImage2.binding = 2;
    accumulationImage2.descriptorCount = 1;
    accumulationImage2.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    accumulationImage2.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
    storageImage,
    accumulationImage,
    accumulationImage2,
    };

    std::array<VkDescriptorBindingFlags, 3> bindingFlags = {
    0, // storageImage (binding 0) no flags
    0, // accumulatiobImage (binding 1) no flags
    0 // accumulatiobImage (binding 2) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Engine::rayTracingDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    Engine::layoutsUsed.push_back(Engine::rayTracingDescriptor.layout);
}

void DescriptorManager::createRayTracingDescriptorPool() {

    if (Engine::rayTracingDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Engine::rayTracingDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[0].descriptorCount = Engine::MAX_FRAMES_IN_FLIGHT;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[1].descriptorCount = Engine::MAX_FRAMES_IN_FLIGHT;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    poolSizes[2].descriptorCount = Engine::MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = Engine::MAX_FRAMES_IN_FLIGHT; //PER FRAME
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Engine::rayTracingDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorManager::createRayTracingDescriptorSets() {

    Engine::rayTracingDescriptor.sets.resize(Engine::MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> layouts(Engine::MAX_FRAMES_IN_FLIGHT, Engine::rayTracingDescriptor.layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Engine::rayTracingDescriptor.pool;
    allocInfo.descriptorSetCount = Engine::MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, Engine::rayTracingDescriptor.sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = Engine::swapChainInfo.imageViews[i];
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet imageWrite{};
        imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageWrite.dstSet = Engine::rayTracingDescriptor.sets[i];
        imageWrite.dstBinding = 0;
        imageWrite.dstArrayElement = 0;
        imageWrite.descriptorCount = 1;
        imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        imageWrite.pImageInfo = &imageInfo;

        VkDescriptorImageInfo accumuImageInfo{};
        accumuImageInfo.imageView = Engine::accumulationImages->views[(i + Engine::MAX_FRAMES_IN_FLIGHT - 1) % Engine::MAX_FRAMES_IN_FLIGHT];
        accumuImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet accumImageWrite{};
        accumImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        accumImageWrite.dstSet = Engine::rayTracingDescriptor.sets[i];
        accumImageWrite.dstBinding = 1;
        accumImageWrite.dstArrayElement = 0;
        accumImageWrite.descriptorCount = 1;
        accumImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        accumImageWrite.pImageInfo = &accumuImageInfo;

        VkDescriptorImageInfo accumuImage2Info{};
        accumuImage2Info.imageView = Engine::accumulationImages->views[i];
        accumuImage2Info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet accumImage2Write{};
        accumImage2Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        accumImage2Write.dstSet = Engine::rayTracingDescriptor.sets[i];
        accumImage2Write.dstBinding = 2;
        accumImage2Write.dstArrayElement = 0;
        accumImage2Write.descriptorCount = 1;
        accumImage2Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        accumImage2Write.pImageInfo = &accumuImage2Info;

        std::array<VkWriteDescriptorSet, 3> writes = {
            imageWrite,
            accumImageWrite,
            accumImage2Write
        };

        vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    }
}

void DescriptorManager::createRaySpheresSetLayout() {
    if (Engine::rayTracingSpheresDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::rayTracingSpheresDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding SSBO{};
    SSBO.binding = 0;
    SSBO.descriptorCount = 1;
    SSBO.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    SSBO.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding vertexSSBO{};
    vertexSSBO.binding = 1;
    vertexSSBO.descriptorCount = 1;
    vertexSSBO.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    vertexSSBO.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding indexSSBO{};
    indexSSBO.binding = 2;
    indexSSBO.descriptorCount = 1;
    indexSSBO.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    indexSSBO.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = {
    SSBO,
    vertexSSBO,
    indexSSBO
    };

    std::array<VkDescriptorBindingFlags, 3> bindingFlags = {
    0, // SSBO (binding 0) no flags
    0, // vertexSSBO (binding 1) no flags
    0 // indexSSBO (binding 2) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Engine::rayTracingSpheresDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    Engine::layoutsUsed.push_back(Engine::rayTracingSpheresDescriptor.layout);
}

void DescriptorManager::createRaySpheresPool() {
    if (Engine::rayTracingSpheresDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Engine::rayTracingSpheresDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 3> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = 1;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Engine::rayTracingSpheresDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorManager::createRaySpheresSets() {
    Engine::rayTracingSpheresDescriptor.sets.resize(1);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Engine::rayTracingSpheresDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &Engine::rayTracingSpheresDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, Engine::rayTracingSpheresDescriptor.sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    std::array<VkWriteDescriptorSet, 3> writes{};
    VkDescriptorBufferInfo ssbo{};
    ssbo.buffer = Engine::rayTracingSpheres.buffer;
    ssbo.offset = 0;
    ssbo.range = Engine::rayTracingSpheres.size;

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = Engine::rayTracingSpheresDescriptor.sets[0];
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &ssbo;

    VkDescriptorBufferInfo vertexSsbo{};
    vertexSsbo.buffer = Engine::rayVertices.buffer;
    vertexSsbo.offset = 0;
    vertexSsbo.range = Engine::rayVertices.size;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = Engine::rayTracingSpheresDescriptor.sets[0];
    writes[1].dstBinding = 1;
    writes[1].dstArrayElement = 0;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[1].descriptorCount = 1;
    writes[1].pBufferInfo = &vertexSsbo;

    VkDescriptorBufferInfo indexSsbo{};
    indexSsbo.buffer = Engine::rayIndices.buffer;
    indexSsbo.offset = 0;
    indexSsbo.range = Engine::rayIndices.size;

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = Engine::rayTracingSpheresDescriptor.sets[0];
    writes[2].dstBinding = 2;
    writes[2].dstArrayElement = 0;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[2].descriptorCount = 1;
    writes[2].pBufferInfo = &indexSsbo;

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void DescriptorManager::createRayLightsSetLayout() {
    if (Engine::rayTracingLightsDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, Engine::rayTracingLightsDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding lightsSSBO{};
    lightsSSBO.binding = 0;
    lightsSSBO.descriptorCount = 1;
    lightsSSBO.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightsSSBO.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
    lightsSSBO,
    };

    std::array<VkDescriptorBindingFlags, 1> bindingFlags = {
    0, // lightsSSBO (binding 0) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &Engine::rayTracingLightsDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    Engine::layoutsUsed.push_back(Engine::rayTracingLightsDescriptor.layout);
}

void DescriptorManager::createRayLightsPool() {
    if (Engine::rayTracingLightsDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(Engine::rayTracingLightsDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &Engine::rayTracingLightsDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorManager::createRayLightsSets() {
    Engine::rayTracingLightsDescriptor.sets.resize(1);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Engine::rayTracingLightsDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &Engine::rayTracingLightsDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, Engine::rayTracingLightsDescriptor.sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    std::array<VkWriteDescriptorSet, 1> writes{};
    VkDescriptorBufferInfo ssbo{};
    ssbo.buffer = Engine::lightSources.buffer;
    ssbo.offset = 0;
    ssbo.range = Engine::lightSources.size;

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = Engine::rayTracingLightsDescriptor.sets[0];
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[0].descriptorCount = 1;
    writes[0].pBufferInfo = &ssbo;

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}
#endif