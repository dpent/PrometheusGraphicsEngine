#include "../headers/workerThread.h"
#include "../headers/gameObjectOperations.h"
#include "../headers/descriptorOperations.h"
#include "../headers/modelOperations.h"
#include "../headers/bufferOperations.h"
#include "../../engine/headers/engine.h"
#include <vulkan/vulkan_core.h>


using namespace Prometheus;

namespace Prometheus{

    WorkerThread::WorkerThread(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface){
        this->thread = std::thread(&WorkerThread::workerLoop, this);
        id=thread.get_id();

        createPoolAndBuffers(device,physicalDevice,surface);
    }

    void WorkerThread::workerLoop(){

        while(alive){

            if(!(jobs.empty())){
                jobsMutex.lock();

                Job* job = std::move(jobs.front());
                jobs.pop();

                jobsMutex.unlock();
                
                doWork(job);
            }else if(alive){
                Engine::workInQueueSemaphore.acquire();

                jobsMutex.lock();
                Engine::queueMutex.lock();

                Engine::threadsAvailable.add(-1);

                jobs.push(std::move(Engine::jobQueue.front()));
                
                Engine::jobQueue.pop();


                //std::cout << "OP ID " << jobs.back()->name() << " grabbed. There are " << Engine::jobQueue.size() << " jobs in queue" << std::endl;
                //Engine::printJobQueue();

                Engine::queueMutex.unlock();
                jobsMutex.unlock();
            }

        }

        while(!jobs.empty()){

            jobsMutex.lock();

            if(jobs.front()->name() == "PrepareForJoinJob") {
                doWork(jobs.front());
            }

            jobs.pop();
            jobsMutex.unlock();
        }
    }

    void WorkerThread::doWork(Job* job){

        job->execute(*this);

        Engine::threadsAvailable.add(1);

        delete job;
    }

    void WorkerThread::detach(){
        thread.detach();
    }

    void WorkerThread::createPoolAndBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface){
        BufferManager::createCommandPool(physicalDevice, surface,device,commandPool);
        BufferManager::createCommandBuffers(device, commandBuffers, commandPool);
    }
}