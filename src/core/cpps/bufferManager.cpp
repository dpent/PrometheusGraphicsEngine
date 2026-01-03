#include "../headers/bufferManager.h"
#include "../headers/engine.h"


void BufferManager::createFrameBuffers(std::vector<VkFramebuffer>& frameBuffers,std::vector<VkImageView>& views,VkExtent2D& extent,VkRenderPass& renderPass, VkImageView& colorView,VkImageView& depthView)
{
    frameBuffers.resize(views.size());

    for (size_t i = 0; i < views.size(); i++) {
        std::array<VkImageView, 3>attachments = {
            colorView,
            depthView,
            views[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(Engine::deviceInfo.logicalDevice, &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void BufferManager::createBuffer(Buffer& bufferStruct, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferStruct.size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Engine::deviceInfo.logicalDevice, &bufferInfo, nullptr, &bufferStruct.buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Engine::deviceInfo.logicalDevice, bufferStruct.buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = DeviceManager::findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(Engine::deviceInfo.logicalDevice, &allocInfo, nullptr, &bufferStruct.memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(Engine::deviceInfo.logicalDevice, bufferStruct.buffer, bufferStruct.memory, 0);
}

void BufferManager::createStagingBuffer(VkDeviceSize size) {

    if (Engine::stagingBuffer.buffer != VK_NULL_HANDLE) {
        Engine::stagingBuffer.destroy();
    }

    Engine::stagingBuffer.size = size;
    BufferManager::createBuffer(Engine::stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

}

void BufferManager::createVertexIndexBuffer(VkDeviceSize size) {
    if (Engine::vertexIndexBuffer.buffer != VK_NULL_HANDLE) {
        Engine::vertexIndexBuffer.destroy();
    }

    if (size > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(size);
    }

    Engine::vertexIndexBuffer.offset = uint32_t(Engine::vertexIndexData.vertices.size() * sizeof(Vertex));
    Engine::vertexIndexBuffer.size = size;

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, Engine::vertexIndexBuffer.offset, 0, &data);
    memcpy(data, Engine::vertexIndexData.vertices.data(), (size_t)(sizeof(Vertex) * Engine::vertexIndexData.vertices.size()));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, Engine::vertexIndexBuffer.offset, sizeof(uint32_t) * Engine::vertexIndexData.indices.size(), 0, &data);
    memcpy(data, Engine::vertexIndexData.indices.data(), (size_t)(sizeof(uint32_t) * Engine::vertexIndexData.indices.size()));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    BufferManager::createBuffer(Engine::vertexIndexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    BufferManager::copyBuffer(Engine::stagingBuffer, Engine::vertexIndexBuffer, size);
}

void BufferManager::copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = BufferManager::beginSingleTimeCommands(Engine::command.pool);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

    BufferManager::endSingleTimeCommands(Engine::command.pool, commandBuffer);
}

VkCommandBuffer BufferManager::beginSingleTimeCommands(VkCommandPool& commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Engine::deviceInfo.logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void BufferManager::endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer) {

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Engine::queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Engine::queues.graphics);

    vkFreeCommandBuffers(Engine::deviceInfo.logicalDevice, commandPool, 1, &commandBuffer);

}

void BufferManager::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex) {
    
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional 
    /*
    -- VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
    -- VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
    -- VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    */
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = Engine::graphicsRenderPass;
    renderPassInfo.framebuffer = Engine::swapChainInfo.frameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Engine::swapChainInfo.extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(Engine::swapChainInfo.extent.width);
    viewport.height = static_cast<float>(Engine::swapChainInfo.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = Engine::swapChainInfo.extent;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    CameraObject* cameraPushConstants = new CameraObject();
    cameraPushConstants->view = Engine::camera.getViewMatrix();
    cameraPushConstants->proj = Engine::camera.getProjectionMatrix();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Engine::graphicsPipeLine.pipeline);

    VkBuffer vertexBuffers[] = { Engine::vertexIndexBuffer.buffer};
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, Engine::vertexIndexBuffer.buffer, Engine::vertexIndexBuffer.offset, VK_INDEX_TYPE_UINT32);


    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        Engine::graphicsPipeLine.layout,
        0,                              // first set
        1,                              // number of sets
        &Engine::graphicsDescriptor.sets[0],     // pointer to descriptor set
        0,
        nullptr
    );

    for (auto object : Engine::gameObjects) {

		cameraPushConstants->objectIndex = object->instanceIndex;

        vkCmdPushConstants(
            commandBuffer,
            Engine::graphicsPipeLine.layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(*cameraPushConstants),
            cameraPushConstants
        );

        vkCmdDrawIndexed(
            commandBuffer, 
            static_cast<uint32_t>(object->mesh->indices.size()), 
            1, 
            object->mesh->indexOffset, 
            object->mesh->vertexOffset, 
            0
        );
    }

    if (Engine::displayGUI) {
        GUIManager::renderGUI(imageIndex);
    }

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandPool ::initialize() {
    
    QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(Engine::deviceInfo.physicalDevice, Engine::surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    buffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    allocInfo.commandPool = pool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;/*
                                                        -- VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution,
                                                        but cannot be called from other command buffers.
                                                        -- VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can
                                                        be called from primary command buffers.
                                                    */
    allocInfo.commandBufferCount = (uint32_t)buffers.size();

    if (vkAllocateCommandBuffers(Engine::deviceInfo.logicalDevice, &allocInfo, buffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Buffer::destroy() {
    vkDestroyBuffer(Engine::deviceInfo.logicalDevice, buffer, nullptr);
    vkFreeMemory(Engine::deviceInfo.logicalDevice, memory, nullptr);
}