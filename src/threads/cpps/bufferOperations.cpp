#include "../headers/bufferOperations.h"

using namespace Prometheus;

namespace Prometheus{


    void updateVertexIndexBuffer(VkDevice& device, VkPhysicalDevice& physicalDevice, 
        VkQueue& graphicsQueue,sem_t* jobDoneSem)
    {
        uint64_t size = BufferManager::remakeVertexIndexVectors(device);

        if( size >=Engine::indexVertexBufferSize)
        {
            BufferManager::createIndexVertexBuffer(device,physicalDevice,graphicsQueue);

        }else{
            BufferManager::updateIndexVertexBuffer(device,physicalDevice,graphicsQueue);
        }
        Engine::recreateVertexIndexBuffer=false;

        sem_post(jobDoneSem);
    }

    void recreateInstanceBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice,sem_t* jobDoneSem){
        
        BufferManager::recreateInstanceBuffers(device,physicalDevice);

        Engine::recreateInstanceBuffer=false;
        sem_post(jobDoneSem);
    }

    void updateInstanceBuffer(uint32_t currentImage){

        BufferManager::updateInstanceBuffer(currentImage);
    }
}