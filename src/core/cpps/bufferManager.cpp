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

void BufferManager::createShadowFrameBuffers(std::vector<VkFramebuffer>& frameBuffers,std::vector<VkImageView>& views,VkExtent2D& extent,VkRenderPass& renderPass) {
    frameBuffers.resize(views.size());

    for (size_t i = 0; i < views.size(); i++) {
        std::array<VkImageView, 1>attachments = {
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

void BufferManager::createStagingBuffer(VkDeviceSize size, Buffer& buffer) {

    if (buffer.buffer != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.buffers.push_back(buffer);
        Engine::garbage.bufferFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    buffer.size = size;
    BufferManager::createBuffer(buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

}

void BufferManager::createVertexIndexBuffer(VkDeviceSize size) {
    if (Engine::vertexIndexBuffer.buffer != VK_NULL_HANDLE) {
        Engine::garbage.lock();
        Engine::garbage.buffers.push_back(Engine::vertexIndexBuffer);
        Engine::garbage.bufferFramesPassed.push_back(0);
        Engine::garbage.unlock();
    }

    if (size > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(size, Engine::stagingBuffer);
    }

    Engine::vertexIndexBuffer.offset = uint32_t(Engine::vertexIndexData.vertices.size() * sizeof(Vertex));
    Engine::vertexIndexBuffer.size = size;

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, Engine::vertexIndexBuffer.offset, 0, &data);
    memcpy(data, Engine::vertexIndexData.vertices.data(), (size_t)(sizeof(Vertex) * Engine::vertexIndexData.vertices.size()));
    memcpy((char*)data + Engine::vertexIndexBuffer.offset, Engine::vertexIndexData.indices.data(), (size_t)(sizeof(uint32_t) * Engine::vertexIndexData.indices.size()));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    BufferManager::createBuffer(Engine::vertexIndexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    BufferManager::copyBuffer(Engine::stagingBuffer, Engine::vertexIndexBuffer, size);
}

void BufferManager::createVertexIndexBufferCheckSize(VkDeviceSize size) {

    if (size > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(size, Engine::stagingBuffer);
    }

    if (size > Engine::vertexIndexBuffer.size) {

        if (Engine::vertexIndexBuffer.buffer != VK_NULL_HANDLE) {
            Engine::garbage.lock();
            Engine::garbage.buffers.push_back(Engine::vertexIndexBuffer);
            Engine::garbage.bufferFramesPassed.push_back(0);
            Engine::garbage.unlock();
        }

        Engine::vertexIndexBuffer.size = size;
        BufferManager::createBuffer(Engine::vertexIndexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    }

    Engine::vertexIndexBuffer.offset = uint32_t(Engine::vertexIndexData.vertices.size() * sizeof(Vertex));

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, Engine::vertexIndexBuffer.offset, 0, &data);
    memcpy(data, Engine::vertexIndexData.vertices.data(), (size_t)(sizeof(Vertex) * Engine::vertexIndexData.vertices.size()));
    memcpy((char*)data + Engine::vertexIndexBuffer.offset, Engine::vertexIndexData.indices.data(), (size_t)(sizeof(uint32_t) * Engine::vertexIndexData.indices.size()));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

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

    BufferManager::recordShadowMapCommands(commandBuffer, imageIndex);

    BufferManager::recordGraphicsPass(commandBuffer, imageIndex);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void BufferManager::recordShadowMapCommands(VkCommandBuffer& commandBuffer, uint32_t& imageIndex) {
    
    VkRenderPassBeginInfo shadowPassInfo{};
    shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadowPassInfo.renderPass = Engine::shadowRenderPass;
    shadowPassInfo.renderArea.offset = { 0,0 };
    shadowPassInfo.renderArea.extent = VkExtent2D{ Engine::shadowRes, Engine::shadowRes };

    std::array<VkClearValue, 1> shadowClearValues{};
    shadowClearValues[0].depthStencil = { 1.0f, 0 };

    shadowPassInfo.clearValueCount = static_cast<uint32_t>(shadowClearValues.size());
    shadowPassInfo.pClearValues = shadowClearValues.data();

    ShadowLight* light = Engine::shadowCreatingLights.head;
    int i = 0;
    while (light != nullptr) {

        shadowPassInfo.framebuffer = Engine::shadowFrameBuffers[i];

        vkCmdBeginRenderPass(commandBuffer, &shadowPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(Engine::shadowRes);
        viewport.height = static_cast<float>(Engine::shadowRes);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = VkExtent2D{ Engine::shadowRes, Engine::shadowRes };

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdSetDepthBias(
            commandBuffer,
            1.25f,
            0.0f,
            1.75f
        );

        LightVPObject* lightPushConstants = new LightVPObject();
        lightPushConstants->lightVP = light->getLightVP();

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Engine::shadowPipeline.pipeline);

        VkBuffer vertexBuffers[] = { Engine::vertexIndexBuffer.buffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, Engine::vertexIndexBuffer.buffer, Engine::vertexIndexBuffer.offset, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            Engine::shadowPipeline.layout,
            0,                              // first set
            1,                              // number of sets
            &Engine::graphicsDescriptor.sets[0],     // pointer to descriptor set
            0,
            nullptr
        );


        GameObject* object = Engine::gameObjects.head;
        while (object != nullptr) {
            lightPushConstants->objectIndex = object->instanceIndex;

            vkCmdPushConstants(
                commandBuffer,
                Engine::shadowPipeline.layout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(*lightPushConstants),
                lightPushConstants
            );

            vkCmdDrawIndexed(
                commandBuffer,
                static_cast<uint32_t>(object->mesh->indices.size()),
                1,
                object->mesh->indexOffset,
                object->mesh->vertexOffset,
                0
            );

            object = object->next;
        }

        vkCmdEndRenderPass(commandBuffer);

        light = light->next;
        i++;

        delete lightPushConstants;
    }
}

void BufferManager::recordGraphicsPass(VkCommandBuffer& commandBuffer, uint32_t& imageIndex) {

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = Engine::graphicsRenderPass;
    renderPassInfo.framebuffer = Engine::swapChainInfo.frameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Engine::swapChainInfo.extent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = Engine::viewportLimitsOffsets.x;
    viewport.y = Engine::viewportLimitsOffsets.y;

    viewport.width =
        static_cast<float>(Engine::swapChainInfo.extent.width
        - Engine::viewportLimitsOffsets.x
        - Engine::viewportLimitsOffsets.z);

    viewport.height =
        static_cast<float>(Engine::swapChainInfo.extent.height
        - Engine::viewportLimitsOffsets.y
        - Engine::viewportLimitsOffsets.w);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {
        (int32_t)viewport.x,
        (int32_t)viewport.y
    };
    scissor.extent = {
        (uint32_t)viewport.width,
        (uint32_t)viewport.height
    };;

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    CameraObject* cameraPushConstants = new CameraObject();
    cameraPushConstants->view = Engine::camera.getViewMatrix();
    cameraPushConstants->proj = Engine::camera.getProjectionMatrix();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Engine::graphicsPipeLine.pipeline);

    VkBuffer vertexBuffers[] = { Engine::vertexIndexBuffer.buffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, Engine::vertexIndexBuffer.buffer, Engine::vertexIndexBuffer.offset, VK_INDEX_TYPE_UINT32);

    std::array<VkDescriptorSet, 2> sets{
        Engine::graphicsDescriptor.sets[0],     // set = 0
        Engine::shadowLightsDescriptor.sets[0]
    };

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        Engine::graphicsPipeLine.layout,
        0,                              // first set
        static_cast<uint32_t>(sets.size()),       // number of sets
        sets.data(),     // pointer to descriptor set
        0,
        nullptr
    );

    GameObject* object = Engine::gameObjects.head;
    while (object != nullptr) {
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

        object = object->next;
    }

    if (Engine::particleEffects.size != 0) {
        BufferManager::recordParticleGraphicsCommands(commandBuffer, imageIndex);
    }

    #ifndef RELEASE
    if (Debug::lines.size() != 0) {
        BufferManager::recordDebugCommands(commandBuffer, imageIndex);
    }
    #endif

    if (Engine::displayGUI) {
        GUIManager::renderGUI(imageIndex);
    }
    else {
        Engine::viewportLimitsOffsets.x = 0.0f;
        Engine::viewportLimitsOffsets.y = 0.0f;
        Engine::viewportLimitsOffsets.z = 0.0f;
        Engine::viewportLimitsOffsets.w = 0.0f;
    }

    vkCmdEndRenderPass(commandBuffer);

    delete cameraPushConstants;
}

void BufferManager::createUniformBuffer(Buffer& buffer, VkDeviceSize size) {
    buffer.size = size;

    createBuffer(buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    vkMapMemory(Engine::deviceInfo.logicalDevice, buffer.memory, 0, size, 0, &buffer.mapped);
}

void BufferManager::recordDebugCommands(VkCommandBuffer& commandBuffer, uint32_t& imageIndex) {

    CameraObject* cameraPushConstants = new CameraObject();
    cameraPushConstants->view = Engine::camera.getViewMatrix();
    cameraPushConstants->proj = Engine::camera.getProjectionMatrix();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Debug::debugPipeline.pipeline);

    VkBuffer vertexBuffers[] = { Debug::debugVertexBuffer.buffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        Debug::debugPipeline.layout,
        0,
        1,
        &Debug::debugDescriptor.sets[0],
        0,
        nullptr
    );

    vkCmdPushConstants(
        commandBuffer,
        Debug::debugPipeline.layout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(*cameraPushConstants),
        cameraPushConstants
    );

    vkCmdDraw(
        commandBuffer,
        static_cast<uint32_t>(Debug::debugVertices.size()),
        static_cast<uint32_t>(Debug::lines.size()),
        0,
        0
    );

    delete cameraPushConstants;
}

void BufferManager::recordParticleGraphicsCommands(VkCommandBuffer& commandBuffer, uint32_t& imageIndex) {
	CameraVPObject* cameraPushConstants = new CameraVPObject();
    cameraPushConstants->view = Engine::camera.getViewMatrix();
    cameraPushConstants->proj = Engine::camera.getProjectionMatrix();

    ParticleEffect* effect = Engine::particleEffects.head;

    while (effect != nullptr) {
    
	    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, effect->graphicsPipeline.pipeline);

        VkBuffer vertexBuffers[] = { effect->buffers[Engine::currentFrame].buffer};
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdPushConstants(
            commandBuffer,
            effect->graphicsPipeline.layout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(*cameraPushConstants),
            cameraPushConstants
        );

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            effect->graphicsPipeline.layout,
            0,
            static_cast<uint32_t>(effect->graphicsDescriptor.sets.size()),
            effect->graphicsDescriptor.sets.data(),
            0,
            nullptr
        );

        vkCmdDraw(commandBuffer, 4, static_cast<uint32_t>(effect->particles.size()), 0, 0);

        effect = effect->next;
    }


    delete cameraPushConstants;

}

void BufferManager::createDebugVertexBuffer(VkDeviceSize size) {

    if (size > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(size, Engine::stagingBuffer);
    }
    Debug::debugVertexBuffer.size = size;

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, Debug::debugVertexBuffer.size, 0, &data);
    memcpy(data, Debug::debugVertices.data(), (size_t)(sizeof(DebugVertex) * Debug::debugVertices.size()));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    BufferManager::createBuffer(Debug::debugVertexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    BufferManager::copyBuffer(Engine::stagingBuffer, Debug::debugVertexBuffer, size);

}

void BufferManager::recordComputeCommandBuffer(VkCommandBuffer& commandBuffer) {

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    BufferManager::recordParticleComputeCommands(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void BufferManager::recordParticleComputeCommands(VkCommandBuffer& commandBuffer) {

    ParticleEffect* effect = Engine::particleEffects.head;

    while (effect != nullptr) {

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, effect->pipeline.pipeline);

        auto pc = effect->getComputePushConstants();

        vkCmdPushConstants(
            commandBuffer,
            effect->pipeline.layout,
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            pc->size(),
		    pc->data()
        );

        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_COMPUTE, 
            effect->pipeline.layout,
            0,
            1, 
            &effect->sets[Engine::currentFrame],
            0, 
            0
        );

        uint32_t particleCount =static_cast<uint32_t>(effect->particles.size());
        const uint32_t localSize = 256;
        uint32_t groupCount = (particleCount + localSize - 1) / localSize;

        vkCmdDispatch(commandBuffer, groupCount, 1, 1);

        effect = effect->next;
    }

}

#ifdef RAY_TRACING
void BufferManager::recordRayTracingCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex) {
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    /*
    -- VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
    -- VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT: This is a secondary command buffer that will be entirely within a single render pass.
    -- VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT: The command buffer can be resubmitted while it is also already pending execution.
    */
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    BufferManager::recordRayTracingComputeCommands(commandBuffer, imageIndex);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void BufferManager::recordRayTracingComputeCommands(VkCommandBuffer& commandBuffer, uint32_t imageIndex) {

    ImageManager::transitionImageLayout(
        Engine::swapChainInfo.images[imageIndex],
        Engine::swapChainInfo.imageFormat,
        Engine::swapChainInfo.imageLayouts[imageIndex],
        VK_IMAGE_LAYOUT_GENERAL,
        1,
        commandBuffer);

    Engine::swapChainInfo.imageLayouts[imageIndex] = VK_IMAGE_LAYOUT_GENERAL;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Engine::rayTracingPipeline.pipeline);

    std::vector<VkDescriptorSet> sets{
        Engine::rayTracingDescriptor.sets[Engine::currentFrame],
        Engine::rayTracingSpheresDescriptor.sets[0]
    };
    
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        Engine::rayTracingPipeline.layout,
        0, static_cast<uint32_t>(sets.size()),
        sets.data(),
        0, nullptr);

    RayTracingCameraObject* cam = Engine::camera.getRayTracingDetails();

    vkCmdPushConstants(
        commandBuffer,
        Engine::rayTracingPipeline.layout,
        VK_SHADER_STAGE_COMPUTE_BIT,
        0,
        sizeof(RayTracingCameraObject),
        cam
    );

    vkCmdDispatch(commandBuffer,
        (Engine::swapChainInfo.extent.width + 8 - 1) / 8,
        (Engine::swapChainInfo.extent.height + 8 - 1) / 8,
        1
    );

    ImageManager::transitionImageLayout(
        Engine::swapChainInfo.images[imageIndex],
        Engine::swapChainInfo.imageFormat,
        Engine::swapChainInfo.imageLayouts[imageIndex],
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        1,
        commandBuffer);

    Engine::swapChainInfo.imageLayouts[imageIndex] = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    delete cam;
}
#endif

void CommandPool ::initialize() {
    
    QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(Engine::deviceInfo.physicalDevice, Engine::surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(Engine::deviceInfo.logicalDevice, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }

    buffers.resize(Engine::swapChainInfo.images.size());
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

std::string Buffer::getSizeHumanReadable(VkDeviceSize size) {
    static const char* suffixes[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    static constexpr size_t suffixCount = sizeof(suffixes) / sizeof(suffixes[0]);

    double value = static_cast<double>(size);
    size_t suffixIndex = 0;

    while (value >= 1024.0 && suffixIndex < suffixCount - 1)
    {
        value /= 1024.0;
        ++suffixIndex;
    }

    std::ostringstream out;
    out << std::fixed << std::setprecision(value < 10.0 ? 2 : 1)
        << value << " " << suffixes[suffixIndex];

    return out.str();
}