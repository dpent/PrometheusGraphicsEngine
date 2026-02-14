#include "../headers/particleEffect.h"
#include "../../core/headers/engine.h"
#include <random>
#include "../../core/headers/stb_image.h"

ParticleEffect::ParticleEffect() {}

ParticleEffect::ParticleEffect(uint64_t numParticles, std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC) {
	
	this->computeShaderFilename = computeShaderFilename;
	this->vertexShaderFilename = vertexShaderFilename;
	this->fragmentShaderFilename = fragmentShaderFilename;
    this->computePushConstant = cPC;

	particles.reserve(numParticles);

	std::default_random_engine gen;
	std::uniform_real_distribution<float> distribution(0.0f, 10.0f);

	for (uint64_t i = 0; i < numParticles; i++) {
		
		glm::vec3 position{ distribution(gen), distribution(gen), distribution(gen) };
		glm::vec3 color{ distribution(gen) / 10.0f, distribution(gen) / 10.0f, distribution(gen) / 10.0f };
		glm::vec3 velocity{ 0.0f };

		Particle p{position,color,velocity};

		particles.push_back(p);
	}

    createBuffers();
}

ParticleEffect::ParticleEffect(std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC) {
	this->computeShaderFilename = computeShaderFilename;
	this->vertexShaderFilename = vertexShaderFilename;
	this->fragmentShaderFilename = fragmentShaderFilename;
    this->computePushConstant = cPC;
}

void ParticleEffect::createGraphicsDescriptor() {

    if (this->graphicsDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, this->graphicsDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {
    samplerBinding,
    //texturesBinding
    };

    std::array<VkDescriptorBindingFlags, 1> bindingFlags = {
    0, // samplerBinding (binding 0) no flags
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &this->graphicsDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }


    if (this->graphicsDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(this->graphicsDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 1> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1; //NOT PER FRAME. ITS 1 GLOBAL
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &this->graphicsDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }


    this->graphicsDescriptor.sets.resize(1);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->graphicsDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &this->graphicsDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, &this->graphicsDescriptor.sets[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = Engine::linearSampler;

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstSet = this->graphicsDescriptor.sets[0];
    samplerWrite.dstBinding = 0;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerWrite.descriptorCount = 1;
    samplerWrite.pImageInfo = &samplerInfo;

    std::array<VkWriteDescriptorSet, 1> writes = {
        //write,
        samplerWrite,
    };

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void ParticleEffect::addParticles(uint64_t numParticles) {

    particles.reserve(numParticles);

    std::default_random_engine gen;
    std::uniform_real_distribution<float> distribution(0.0f, 10.0f);

    for (uint64_t i = 0; i < numParticles; i++) {

        glm::vec3 position{ distribution(gen), distribution(gen), distribution(gen) };
        glm::vec3 color{ distribution(gen) / 10.0f, distribution(gen) / 10.0f, distribution(gen) / 10.0f };
        glm::vec3 velocity{ 0.0f };

        Particle p{ position,color,velocity };

        particles.push_back(p);
    }

    createBuffers();
}

void ParticleEffect::createBuffers() {
    buffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize bufferSize = sizeof(Particle) * particles.size();


    if (bufferSize > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(bufferSize, Engine::stagingBuffer);
    }

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, bufferSize, 0, &data);
    memcpy(data, particles.data(), bufferSize);
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    for (size_t i = 0; i < buffers.size(); i++) {

        BufferManager::createParticleSSBO(buffers[i], Engine::stagingBuffer, particles);
        BufferManager::copyBuffer(Engine::stagingBuffer, buffers[i], bufferSize);
    }
}

void ParticleEffect::createGraphicsPipeline() {
    auto vertShaderCode = Engine::readFile((std::filesystem::path(BIN_DIR) / vertexShaderFilename).string());
    auto fragShaderCode = Engine::readFile((std::filesystem::path(BIN_DIR) / fragmentShaderFilename).string());

    VkShaderModule vertShaderModule = Engine::createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = Engine::createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo shaderStages[] =
    {
        Engine::createShaderStageInfo( //Vertex
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            VK_SHADER_STAGE_VERTEX_BIT,
            vertShaderModule,
            "main",
            nullptr
        ),
        Engine::createShaderStageInfo( //Fragment
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            VK_SHADER_STAGE_FRAGMENT_BIT,
            fragShaderModule,
            "main",
            nullptr
        )
    };

    auto bindingDescription = Particle::getBindingDescription();
    auto attributeDescriptions = Particle::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingDescription.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = Engine::swapChainInfo.extent;

    //Dynamic viewport and scissor if needed
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = Engine::msaaSamples;
    multisampling.sampleShadingEnable = VK_FALSE; // VK_TRUE to enable sample shading in the pipeline
    multisampling.minSampleShading = 1.0f; //0.2f is the min fraction for sample shading, closer to one is smoother
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;
    depthStencil.maxDepthBounds = 1.0f;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // accessible in vertex shader
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(glm::mat4) * 2;

    std::array<VkDescriptorSetLayout, 1> setLayouts = {
    graphicsDescriptor.layout,      // set = 0
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(Engine::deviceInfo.logicalDevice, &pipelineLayoutInfo, nullptr, &graphicsPipeline.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{}; //Gather all structs for the pipeline creation
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = graphicsPipeline.layout;
    pipelineInfo.renderPass = Engine::graphicsRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional
    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(Engine::deviceInfo.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(Engine::deviceInfo.logicalDevice, fragShaderModule, nullptr);
    vkDestroyShaderModule(Engine::deviceInfo.logicalDevice, vertShaderModule, nullptr);
}

void ParticleEffect::createComputeDescriptorSets(){

    this->sets.resize(Engine::MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> layouts(Engine::MAX_FRAMES_IN_FLIGHT, Engine::particleDescriptor.layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = Engine::particleDescriptor.pool;
    allocInfo.descriptorSetCount = Engine::MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, this->sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }


    for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {

        std::array<VkWriteDescriptorSet, 2> writes{};
        VkDescriptorBufferInfo storageBufferInfoLastFrame{};
        storageBufferInfoLastFrame.buffer = this->buffers[(i - 1) % Engine::MAX_FRAMES_IN_FLIGHT].buffer;
        storageBufferInfoLastFrame.offset = 0;
        storageBufferInfoLastFrame.range = this->buffers[(i - 1) % Engine::MAX_FRAMES_IN_FLIGHT].size;

        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = this->sets[i];
        writes[0].dstBinding = 0;
        writes[0].dstArrayElement = 0;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[0].descriptorCount = 1;
        writes[0].pBufferInfo = &storageBufferInfoLastFrame;

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
        storageBufferInfoCurrentFrame.buffer = this->buffers[i].buffer;
        storageBufferInfoCurrentFrame.offset = 0;
        storageBufferInfoCurrentFrame.range = this->buffers[i].size;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = this->sets[i];
        writes[1].dstBinding = 1;
        writes[1].dstArrayElement = 0;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[1].descriptorCount = 1;
        writes[1].pBufferInfo = &storageBufferInfoCurrentFrame;

        vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, 2, writes.data(), 0, nullptr);
    }
}

void ParticleEffect::createComputePipeline() {
    auto compShaderCode = Engine::readFile((std::filesystem::path(BIN_DIR) / this->computeShaderFilename).string());
    VkShaderModule compShaderModule = Engine::createShaderModule(compShaderCode);

    VkPipelineShaderStageCreateInfo compShaderStage = Engine::createShaderStageInfo( //Vertex
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        VK_SHADER_STAGE_COMPUTE_BIT,
        compShaderModule,
        "main",
        nullptr
    );

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &Engine::particleDescriptor.layout;

    if (vkCreatePipelineLayout(Engine::deviceInfo.logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipeline.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = this->pipeline.layout;
    pipelineInfo.stage = compShaderStage;

    if (vkCreateComputePipelines(Engine::deviceInfo.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(Engine::deviceInfo.logicalDevice, compShaderModule, nullptr);
}

void ParticleEffect::update() {
    computePushConstant->update();
    return;
}

ComputePushContant* ParticleEffect::getComputePushConstants() {
    return computePushConstant;
}

ComputePushContant::ComputePushContant() {
    return;
}

void ComputePushContant::update() {
    return;
}

/// <summary>
/// SMOKE EFFECT #######################################################################################################
/// </summary>
SmokeEffect::SmokeEffect(){}

SmokeEffect::SmokeEffect(uint64_t numParticles, std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC)
    : ParticleEffect(numParticles, computeShaderFilename, vertexShaderFilename, fragmentShaderFilename, cPC)
{

    this->createComputeDescriptorSets();
    this->createComputePipeline();
    this->loadImage();
    this->createGraphicsDescriptor();
    this->createGraphicsPipeline();
}

SmokeEffect::SmokeEffect(std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC)
    : ParticleEffect(computeShaderFilename, vertexShaderFilename, fragmentShaderFilename, cPC)
{
    this->loadImage();
}

void SmokeEffect::createGraphicsDescriptor() {
    if (this->graphicsDescriptor.layout != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(Engine::deviceInfo.logicalDevice, this->graphicsDescriptor.layout, nullptr);
    }

    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 0;
    samplerBinding.descriptorCount = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding texturesBinding{};
    texturesBinding.binding = 1;
    texturesBinding.descriptorCount = 1;
    texturesBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    texturesBinding.pImmutableSamplers = nullptr;
    texturesBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {
    samplerBinding,
    texturesBinding
    };

    std::array<VkDescriptorBindingFlags, 2> bindingFlags = {
    0, // samplerBinding (binding 0) no flags
    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT // texturesBinding (binding 1)
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

    if (vkCreateDescriptorSetLayout(Engine::deviceInfo.logicalDevice, &layoutInfo, nullptr, &this->graphicsDescriptor.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }


    if (this->graphicsDescriptor.pool != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.descriptors.push_back(this->graphicsDescriptor.pool);
        Engine::garbage.descriptorFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    std::array<VkDescriptorPoolSize, 2> poolSizes{};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[1].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.maxSets = 1; //NOT PER FRAME. ITS 1 GLOBAL
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &this->graphicsDescriptor.pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }


    this->graphicsDescriptor.sets.resize(1);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->graphicsDescriptor.pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &this->graphicsDescriptor.layout;

    if (vkAllocateDescriptorSets(Engine::deviceInfo.logicalDevice, &allocInfo, &this->graphicsDescriptor.sets[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor set!");
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = image.view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo samplerInfo{};
    samplerInfo.sampler = Engine::linearSampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = this->graphicsDescriptor.sets[0];
    write.dstBinding = 1;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    VkWriteDescriptorSet samplerWrite{};
    samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    samplerWrite.dstSet = this->graphicsDescriptor.sets[0];
    samplerWrite.dstBinding = 0;
    samplerWrite.dstArrayElement = 0;
    samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerWrite.descriptorCount = 1;
    samplerWrite.pImageInfo = &samplerInfo;

    std::array<VkWriteDescriptorSet, 2> writes = {
        write,
        samplerWrite,
    };

    vkUpdateDescriptorSets(Engine::deviceInfo.logicalDevice, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void SmokeEffect::loadImage() {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load((std::filesystem::path(TEXTURE_DIR) / "smoke_rising.png").lexically_normal().string().c_str(), &texWidth, &texHeight, &texChannels, 4);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {

        const char* error = stbi_failure_reason();
        std::cout << "STBI Error: " << (error ? error : "Unknown error") << std::endl;
        std::cout << "Failed to load: " << "smoke_rising.png" << std::endl;

        throw std::runtime_error("failed to load texture image!");
    }

    if (imageSize > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(imageSize, Engine::stagingBuffer);
    }

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    stbi_image_free(pixels);

    ImageManager::createImage(texWidth,
        texHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->image.image,
        this->image.memory,
        mipLevels,
        VK_SAMPLE_COUNT_1_BIT
    );

    this->mipLevels = mipLevels;

    Engine::textureMutex.lock();
    ImageManager::transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, Engine::command.pool);
    ImageManager::copyBufferToImage(Engine::stagingBuffer.buffer, image.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
        Engine::command.pool);

    ImageManager::generateMipMaps(image.image, texWidth, texHeight, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, Engine::command.pool);

    ImageManager::createImageView(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, VK_IMAGE_VIEW_TYPE_2D, 1, 0, image.view);

    Engine::textureMutex.unlock();
}

void SmokeEffect::addParticles(uint64_t numParticles) {

    particles.reserve(numParticles);

    std::default_random_engine gen;
    std::uniform_real_distribution<float> distribution(0.0f, 2.0f);

    for (uint64_t i = 0; i < numParticles; i++) {

        glm::vec3 position{ distribution(gen), distribution(gen), distribution(gen) };
        glm::vec3 color{ distribution(gen) / 2.0f, distribution(gen) / 2.0f, distribution(gen) / 2.0f };
        glm::vec3 velocity{ 0.0f, 0.1f, 0.0f };

        Particle p{ position,color,velocity };

        particles.push_back(p);
    }

    this->createGraphicsDescriptor();
    this->createGraphicsPipeline();
    createBuffers();
    this->createComputeDescriptorSets();
    this->createComputePipeline();
}

void SmokeEffect::createGraphicsPipeline(){

    ParticleEffect::createGraphicsPipeline();
}

void SmokeEffect::createComputeDescriptorSets() {

    ParticleEffect::createComputeDescriptorSets();

}

void SmokeEffect::createComputePipeline() {

    auto compShaderCode = Engine::readFile((std::filesystem::path(BIN_DIR) / this->computeShaderFilename).string());
    VkShaderModule compShaderModule = Engine::createShaderModule(compShaderCode);

    VkPipelineShaderStageCreateInfo compShaderStage = Engine::createShaderStageInfo( //Vertex
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        VK_SHADER_STAGE_COMPUTE_BIT,
        compShaderModule,
        "main",
        nullptr
    );

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(SmokeComputePushConstant);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &Engine::particleDescriptor.layout;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    if (vkCreatePipelineLayout(Engine::deviceInfo.logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipeline.layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = this->pipeline.layout;
    pipelineInfo.stage = compShaderStage;

    if (vkCreateComputePipelines(Engine::deviceInfo.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->pipeline.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(Engine::deviceInfo.logicalDevice, compShaderModule, nullptr);

}

void SmokeEffect::update() {
    computePushConstant->update();
    return;
}

SmokeComputePushConstant::SmokeComputePushConstant(float maxHeight, float windSpeed, glm::vec4 emitterPosition, glm::vec4 windDirection) {
     

    SmokePCData data{
        .maxHeight = maxHeight,
        .windSpeed = windSpeed,
        .emitterPosition = emitterPosition,
        .windDirection = windDirection
    };

    this->pc = data;
}

const void* SmokeComputePushConstant::data() {
    return &pc;
}

const uint32_t SmokeComputePushConstant::size(){
    return sizeof(SmokePCData);
}

void SmokeComputePushConstant::update() {

    /*float time = static_cast<float>(glfwGetTime());

    float windChangeSpeed = 0.5f; // how fast wind direction changes
    float windMaxAngle = glm::radians(30.0f);

    float yawOffset = sin(time * windChangeSpeed) * windMaxAngle;
    float pitchOffset = cos(time * windChangeSpeed * 0.7f) * (windMaxAngle * 0.5f);

    glm::vec3 dir = glm::rotateY(pc.windDirection, yawOffset);
    dir = glm::rotateX(dir, pitchOffset);

    pc.windDirection = glm::vec4(glm::normalize(dir), 0.0f);*/
    pc.windDirection = pc.windDirection;
}