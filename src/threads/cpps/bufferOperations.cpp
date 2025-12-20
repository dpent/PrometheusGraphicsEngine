#include "../headers/bufferOperations.h"

using namespace Prometheus;

namespace Prometheus{


    void updateVertexIndexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, 
        VkQueue& graphicsQueue, VkCommandPool& commandPool)
    {
        uint64_t size = BufferManager::remakeVertexIndexVectors(device);
        if( size >=Engine::indexVertexBufferSize)
        {
            BufferManager::createIndexVertexBuffer(device,physicalDevice,graphicsQueue, commandPool);

        }else{
            BufferManager::updateIndexVertexBuffer(device,physicalDevice,graphicsQueue, commandPool);
        }
        Engine::recreateVertexIndexBuffer=false;

        Engine::verIndBufferComplete.release();
    }

    void recreateInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice, std::binary_semaphore* jobDoneSem){
        
        BufferManager::recreateInstanceBuffers(device,physicalDevice);

        Engine::recreateInstanceBuffer=false;
        jobDoneSem->release();
    }

    void updateInstanceBuffer(uint32_t currentImage){

        BufferManager::updateInstanceBuffer(currentImage);
    }

    void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex,
        VkDevice& device, VkPhysicalDevice& physicalDevice){

        Engine::commandPoolMutex.lock();
        BufferManager::recordCommandBuffer(commandBuffer, imageIndex,device,
        physicalDevice);
        Engine::commandPoolMutex.unlock();
    }

    void cleanup(VkDevice& device, VkCommandPool& commandPool){

        vkDestroyCommandPool(device, commandPool, nullptr);
    }
}