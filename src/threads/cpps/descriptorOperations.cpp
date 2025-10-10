#include "../../engine/headers/engine.h"
#include "../headers/descriptorOperations.h"

using namespace Prometheus;

namespace Prometheus{
    void updateDescriptorDeleteQueue(VkDevice &device){

        Engine::descriptorQueuedMutex.lock();

        auto it1=Engine::descriptorDeleteQueue.begin();
        auto it2=Engine::framesSinceDescriptorQueuedForDeletion.begin();

        while(it1 != Engine::descriptorDeleteQueue.end()){

            (*it2)++;

            if(*it2 == Engine::MAX_FRAMES_IN_FLIGHT){

                if(*it1 != VK_NULL_HANDLE){
                    vkDestroyDescriptorPool(device, *it1, nullptr);
                }

                it1 = Engine::descriptorDeleteQueue.erase(it1);
                it2 = Engine::framesSinceDescriptorQueuedForDeletion.erase(it2); 
            }else{
                it1++;
                it2++;
            }
        }

        Engine::descriptorQueuedMutex.unlock();
    }

    void recreateDescriptorSetsAndPool(VkDevice& device, sem_t* jobDoneSem){

        Engine::graphicsQueueMutex.lock();

        vkDeviceWaitIdle(device);

        Engine::graphicsQueueMutex.unlock();

        Engine::descriptorQueuedMutex.lock();

        Engine::descriptorDeleteQueue.push_back(std::move(Engine::descriptorPool));
        Engine::framesSinceDescriptorQueuedForDeletion.push_back(0);

        Engine::descriptorQueuedMutex.unlock();

        //Engine::meshMutex.lock();
        if(Engine::meshBatches.size()==0){
            sem_post(&Engine::descriptorsReadySemaphore);
            return;
        }
        //Engine::meshMutex.unlock();

        DescriptorManager::createDescriptorPool(device);
        DescriptorManager::createDescriptorSets(device);

        sem_post(jobDoneSem);
    }
}