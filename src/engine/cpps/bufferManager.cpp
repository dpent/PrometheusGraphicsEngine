#include "../headers/bufferManager.h"
#include "../headers/engine.h"
#include "../headers/queueManager.h"
#include <vulkan/vulkan_core.h>
#include "../headers/swapChainManager.h"


using namespace Prometheus;

namespace Prometheus{
    void BufferManager::createFrameBuffers(VkDevice& device){
        Engine::swapChainFramebuffers.resize(Engine::swapChainImageViews.size());

        for (size_t i = 0; i < Engine::swapChainImageViews.size(); i++) {
            std::array<VkImageView, 3> attachments = {
                Engine::colorImageView,
                Engine::depthImageView,
                Engine::swapChainImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = Engine::renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());;
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = Engine::swapChainExtent.width;
            framebufferInfo.height = Engine::swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &Engine::swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void BufferManager::createCommandPool(VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface, VkDevice& device){
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(device, &poolInfo, nullptr, &Engine::commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void BufferManager::createCommandBuffers(VkDevice& device){
        Engine::commandBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        
        Engine::commandPoolMutex.lock();
        allocInfo.commandPool = Engine::commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;/*
                                                            -- VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution,
                                                            but cannot be called from other command buffers.
                                                            -- VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can
                                                            be called from primary command buffers.
                                                        */
        allocInfo.commandBufferCount = (uint32_t) Engine::commandBuffers.size();

        if (vkAllocateCommandBuffers(device, &allocInfo, Engine::commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        Engine::commandPoolMutex.unlock();
    }

    void BufferManager::recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex,
        VkDevice& device, VkPhysicalDevice& physicalDevice){
        
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
        renderPassInfo.renderPass = Engine::renderPass;
        renderPassInfo.framebuffer = Engine::swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = Engine::swapChainExtent;

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}}; //This is the background color. When nothing is drawn it defaults to this
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        /* 
        -- VK_SUBPASS_CONTENTS_INLINE: The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
        -- VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS: The render pass commands will be executed from secondary command buffers.
        */

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Engine::graphicsPipeline);

        sem_wait(&Engine::instanceBufferReady);
        Engine::gameObjectMutex.lock();
        if(Engine::gameObjectMap.size()!=0 ){
            
            Engine::gameObjectMutex.unlock();

            BufferManager::updateInstanceBuffer(Engine::currentFrame);

            VkBuffer vertexBuffers[] = {Engine::indexVertexBuffer,Engine::instanceBuffers[Engine::currentFrame]};
            VkDeviceSize offsets[] = {0,0};

            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, Engine::indexVertexBuffer, Engine::indexOffset, VK_INDEX_TYPE_UINT32);
        }else{
            Engine::gameObjectMutex.unlock();
        }

        VkViewport viewport{}; //Viewport and Scissor was set to dynamic in createGraphicsPipeline (graphicsPipeline.cpp)
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(Engine::swapChainExtent.width);
        viewport.height = static_cast<float>(Engine::swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = Engine::swapChainExtent;

        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        Engine::view = glm::lookAt(
            glm::vec3(15.0f, 10.0f, 15.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        );
        Engine::proj=glm::perspective(
            glm::radians(45.0f),
            Engine::swapChainExtent.width / 
            (float) Engine::swapChainExtent.height, 0.1f, 100.0f
        );
        Engine::proj[1][1] *= -1;

        CameraObject* cameraPushConstants= new CameraObject();
        cameraPushConstants->view=Engine::view;
        cameraPushConstants->proj=Engine::proj;


        vkCmdPushConstants(
            commandBuffer,
            Engine::pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(*cameraPushConstants),
            cameraPushConstants
        );

        uint32_t instanceCount=0;
        for(uint32_t i=0; i<Engine::meshBatches.size(); i++){

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                Engine::pipelineLayout,
                0,                              // first set
                1,                              // number of sets
                &Engine::descriptorSets[i],     // pointer to descriptor set
                0,
                nullptr
            );
            Engine::meshBatches[i].objects[0]->draw(commandBuffer,Engine::meshBatches[i].instances.size(),instanceCount);
            instanceCount+=Engine::meshBatches[i].instances.size();
        }

        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

        delete cameraPushConstants;
    }

    void BufferManager::createIndexVertexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue){

        VkDeviceSize bufferSize = (sizeof(Engine::vertices[0]) * Engine::vertices.size())+(sizeof(Engine::indices[0]) * Engine::indices.size());
        bufferSize = bufferSize<<1;
        Engine::indexVertexBufferSize = bufferSize;
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        BufferManager::createJointBuffer(sizeof(Engine::vertices[0]) * Engine::vertices.size(), 
            sizeof(Engine::indices[0]) * Engine::indices.size(), Engine::indexOffset,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, stagingBufferMemory,device,physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, Engine::indexOffset, 0, &data);
        memcpy(data, Engine::vertices.data(), (size_t) (sizeof(Engine::vertices[0]) * Engine::vertices.size()));
        vkUnmapMemory(device, stagingBufferMemory);

        vkMapMemory(device, stagingBufferMemory, Engine::indexOffset, bufferSize-Engine::indexOffset, 0, &data);
        memcpy(data, Engine::indices.data(), (size_t) (sizeof(Engine::indices[0]) * Engine::indices.size()));
        vkUnmapMemory(device, stagingBufferMemory);

        BufferManager::createJointBuffer(sizeof(Engine::vertices[0]) * Engine::vertices.size(), 
            sizeof(Engine::indices[0]) * Engine::indices.size(), Engine::indexOffset,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            Engine::indexVertexBuffer, Engine::indexVertexBufferMemory,device,physicalDevice);

        copyBuffer(stagingBuffer, Engine::indexVertexBuffer, bufferSize,device,graphicsQueue);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
    }

    void BufferManager::updateIndexVertexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue){
        
        VkDeviceSize bufferSize = (sizeof(Engine::vertices[0]) * Engine::vertices.size())+(sizeof(Engine::indices[0]) * Engine::indices.size());

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        BufferManager::createJointBuffer(sizeof(Engine::vertices[0]) * Engine::vertices.size(), 
            sizeof(Engine::indices[0]) * Engine::indices.size(), Engine::indexOffset,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, stagingBufferMemory,device,physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, Engine::indexOffset, 0, &data);
        memcpy(data, Engine::vertices.data(), (size_t) (sizeof(Engine::vertices[0]) * Engine::vertices.size()));
        vkUnmapMemory(device, stagingBufferMemory);

        vkMapMemory(device, stagingBufferMemory, Engine::indexOffset, bufferSize-Engine::indexOffset, 0, &data);
        memcpy(data, Engine::indices.data(), (size_t) (sizeof(Engine::indices[0]) * Engine::indices.size()));
        vkUnmapMemory(device, stagingBufferMemory);

        copyBuffer(stagingBuffer, Engine::indexVertexBuffer, bufferSize,device,graphicsQueue);

        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);

    }

    uint32_t BufferManager::findMemoryType(uint32_t& typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice& physicalDevice){
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, 
        VkDevice& device, VkPhysicalDevice& physicalDevice)
    {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties,physicalDevice);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void BufferManager::createJointBuffer(VkDeviceSize size1, VkDeviceSize size2, VkDeviceSize& offset1, 
        VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, 
        VkDevice& device, VkPhysicalDevice& physicalDevice)
    {

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = (size1 + size2)<<1;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        offset1=size1;

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties,physicalDevice);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }

    void BufferManager::copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, VkDevice& device, VkQueue& graphicsQueue) {
        
        VkCommandBuffer commandBuffer = BufferManager::beginSingleTimeCommands(device);

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0; // Optional
        copyRegion.dstOffset = 0; // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        
        BufferManager::endSingleTimeCommands(commandBuffer,device,graphicsQueue);
    }

    void BufferManager::createUniformBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice){
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        
        Engine::uniformBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::uniformBuffersMemory.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::uniformBuffersMapped.resize(Engine::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                Engine::uniformBuffers[i], Engine::uniformBuffersMemory[i], device, physicalDevice);

            vkMapMemory(device, Engine::uniformBuffersMemory[i], 0, bufferSize, 0, &Engine::uniformBuffersMapped[i]);
        }
    }
    // A more efficient way to pass a small buffer of data to shaders are push constants.
    void BufferManager::updateUniformBuffer(uint32_t currentImage){
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        ubo.proj = glm::perspective(glm::radians(45.0f), Engine::swapChainExtent.width / (float) Engine::swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1; //Image will be flipped if this is deleted since glm was originaly made for openGL where the y coordinate is flipped.

        memcpy(Engine::uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    VkCommandBuffer BufferManager::beginSingleTimeCommands(VkDevice& device) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        
        Engine::commandPoolMutex.lock();
        allocInfo.commandPool = Engine::commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        Engine::commandPoolMutex.unlock();

        return commandBuffer;
    }

    void BufferManager::endSingleTimeCommands(VkCommandBuffer& commandBuffer, VkDevice& device, VkQueue& graphicsQueue) {

        Engine::commandPoolMutex.lock();
        vkEndCommandBuffer(commandBuffer);
        Engine::commandPoolMutex.unlock();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        Engine::graphicsQueueMutex.lock();

        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);

        Engine::graphicsQueueMutex.unlock();

        Engine::commandPoolMutex.lock();

        vkFreeCommandBuffers(device, Engine::commandPool, 1, &commandBuffer);

        Engine::commandPoolMutex.unlock();
    }

    void BufferManager::createInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice){

        Engine::instanceBuffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBufferMemories.resize(Engine::MAX_FRAMES_IN_FLIGHT);
        Engine::instanceBuffersMapped.resize(Engine::MAX_FRAMES_IN_FLIGHT);

        VkDeviceSize bufferSize=0;
        for(uint32_t i=0; i<Engine::meshBatches.size(); i++){
            bufferSize+=sizeof(InstanceInfo) * Engine::meshBatches[i].instances.size();
        }

        bufferSize = bufferSize << 1;
        Engine::instanceBufferSize = bufferSize;

        for(size_t i=0; i<Engine::MAX_FRAMES_IN_FLIGHT; i++){
            BufferManager::createBuffer(bufferSize, 
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            Engine::instanceBuffers[i], Engine::instanceBufferMemories[i],device,physicalDevice);

            vkMapMemory(device, Engine::instanceBufferMemories[i], 0, bufferSize, 0, &Engine::instanceBuffersMapped[i]);
        }
    }

    void BufferManager::updateInstanceBuffer(uint32_t currentImage){

        char* dst = reinterpret_cast<char*>(Engine::instanceBuffersMapped[currentImage]);
        size_t offset = 0;
        for (auto &batch : Engine::meshBatches) {
            size_t batchSize = batch.instances.size() * sizeof(InstanceInfo);
            memcpy(dst + offset, batch.instances.data(), batchSize);
            offset += batchSize;
        }
    }

    void BufferManager::createDepthResources(VkDevice& device,VkPhysicalDevice& physicalDevice){

        VkFormat depthFormat=BufferManager::findDepthFormat(physicalDevice);

        TextureManager::createImage(Engine::swapChainExtent.width, 
            Engine::swapChainExtent.height, depthFormat, 
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Engine::depthImage, 
            Engine::depthImageMemory, device, physicalDevice,1,Engine::msaaSamples
        );

        Engine::depthImageView=SwapChainManager::createImageView(device,Engine::depthImage,depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT,1);
        //No need for transitioning this image since the render pass will take care of it
    }

    /*
    VK_FORMAT_D32_SFLOAT: 32-bit float for depth
    VK_FORMAT_D32_SFLOAT_S8_UINT: 32-bit signed float for depth and 8 bit stencil component
    VK_FORMAT_D24_UNORM_S8_UINT: 24-bit float for depth and 8 bit stencil component
    */

    VkFormat BufferManager::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features,
        VkPhysicalDevice& physicalDevice){

        for (VkFormat format : candidates) {

            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat BufferManager::findDepthFormat(VkPhysicalDevice& physicalDevice){
        
        return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT,
        physicalDevice);
    }

    bool BufferManager::hasStencilComponent(VkFormat format){
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    uint64_t BufferManager::remakeVertexIndexVectors(VkDevice& device){
        Engine::vertices.clear();
        Engine::indices.clear();

        Engine::graphicsQueueMutex.lock();

        vkDeviceWaitIdle(device);

        Engine::graphicsQueueMutex.unlock();

        for (auto& [meshName, mesh] : Engine::meshMap) {
            mesh.vertexOffset=Engine::vertices.size();
            mesh.indexOffset=Engine::indices.size();
            Engine::vertices.insert(Engine::vertices.end(),mesh.vertices.begin(),mesh.vertices.end());
            Engine::indices.insert(Engine::indices.end(),mesh.indices.begin(),mesh.indices.end());
        }

        uint64_t size = (sizeof(Engine::vertices[0]) * Engine::vertices.size())+(sizeof(Engine::indices[0]) * Engine::indices.size());
        
        if( size >=Engine::indexVertexBufferSize){
            
            if (Engine::indexVertexBuffer != VK_NULL_HANDLE) {
                vkDestroyBuffer(device, Engine::indexVertexBuffer, nullptr);
            }
            if (Engine::indexVertexBufferMemory != VK_NULL_HANDLE) {
                vkFreeMemory(device, Engine::indexVertexBufferMemory, nullptr);
            }
        }

        return size;
    }

    void BufferManager::recreateInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice){
        
        Engine::graphicsQueueMutex.lock();

        vkDeviceWaitIdle(device);

        Engine::graphicsQueueMutex.unlock();

        for(size_t i = 0; i < Engine::MAX_FRAMES_IN_FLIGHT; i++){
            // Check before destroying
            if (Engine::instanceBuffers[i] != VK_NULL_HANDLE) {
                vkDestroyBuffer(device, Engine::instanceBuffers[i], nullptr);
            }
            if (Engine::instanceBufferMemories[i] != VK_NULL_HANDLE) {
                vkFreeMemory(device, Engine::instanceBufferMemories[i], nullptr);
            }
        }

        BufferManager::createInstanceBuffers(device,physicalDevice);

        sem_post(&Engine::instanceBufferReady);
    }

    void BufferManager::createColorResources(VkDevice& device, VkPhysicalDevice& physicalDevice){
        VkFormat colorFormat = Engine::swapChainImageFormat;

    TextureManager::createImage(Engine::swapChainExtent.width, Engine::swapChainExtent.height, colorFormat,VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, Engine::colorImage, Engine::colorImageMemory, device,
        physicalDevice, 1, Engine::msaaSamples);

    Engine::colorImageView = SwapChainManager::createImageView(device,Engine::colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    }

}