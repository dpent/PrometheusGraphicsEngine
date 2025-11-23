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

                Job job = std::move(jobs.front());
                jobs.pop();

                jobsMutex.unlock();
                
                doWork(&job);
            }else if(alive){
                sem_wait(&(Engine::workInQueueSemaphore));

                jobsMutex.lock();
                Engine::queueMutex.lock();

                Engine::threadsAvailable.add(-1);

                jobs.push(std::move(Engine::jobQueue.front()));
                
                Engine::jobQueue.pop();

                Engine::queueMutex.unlock();
                jobsMutex.unlock();
            }

        }

        while(!jobs.empty()){

            jobsMutex.lock();

            if(jobs.front().opId == PREPARE_FOR_JOIN){
                doWork(&jobs.front());
            }

            jobs.pop();
            jobsMutex.unlock();
        }
    }

    void WorkerThread::doWork(Job* job){

        switch (job->opId){
            case CREATE_OBJECT:
                createObject(std::get<std::string>(job->data[0]), 
                    std::get<std::string>(job->data[1]), 
                    *std::get<VkDevice*>(job->data[2]), 
                    *std::get<VkPhysicalDevice*>(job->data[3]), 
                    *std::get<VkQueue*>(job->data[4]),
                    commandPool
                );
                break;
            
            case DELETE_OBJECT:
                deleteObject(std::get<GameObject*>(job->data[0]),
                    *std::get<VkDevice*>(job->data[1])
                );
                break;
            
            case UPDATE_TEXTURE_DELETE_QUEUE:
                updateTextureDeleteQueue(*std::get<VkDevice*>(job->data[0]));
                break;

            case LOAD_MODEL:
                loadModel(std::get<std::string>(job->data[0]),
                *std::get<sem_t*>(job->data[1]));
                break;

            case UPDATE_MESH_DATA_STRUCTURES:
                removeUnusedMeshes();
                break; 

            case UPDATE_DESCRIPTOR_DELETE_QUEUE:
                updateDescriptorDeleteQueue(*std::get<VkDevice*>(job->data[0]));
                break;

            case RECREATE_DESCRIPTORS:
                recreateDescriptorSetsAndPool(*std::get<VkDevice*>(job->data[0]),
                    std::get<sem_t*>(job->data[1]));
                break;

            case UPDATE_GAME_OBJECTS:
                updateGameObjects(
                    std::get<Latch*>(job->data[0]),
                    std::get<sem_t*>(job->data[1])
                );
                break;
            
            case UPDATE_OBJECTS_AND_DESCRIPTORS:
                updateObjectsAndDescriptors(*std::get<VkDevice*>(job->data[0]),
                    std::get<sem_t*>(job->data[1]),
                    std::get<sem_t*>(job->data[2]),
                    std::get<Latch*>(job->data[3]),
                    std::get<sem_t*>(job->data[4])
                );
                break;

            case UPDATE_VERTEX_INDEX_BUFFER:
                updateVertexIndexBuffer(*std::get<VkDevice*>(job->data[0]),
                    *std::get<VkPhysicalDevice*>(job->data[1]), 
                    *std::get<VkQueue*>(job->data[2]),
                    commandPool
                );
                break;

            case UPDATE_INSTANCE_BUFFER:
                updateInstanceBuffer(std::get<uint64_t>(job->data[0]));
                break;
                
            case MAKE_INSTANCE_BUFFER:
                recreateInstanceBuffers(*std::get<VkDevice*>(job->data[0]),
                    *std::get<VkPhysicalDevice*>(job->data[1]),
                    std::get<sem_t*>(job->data[2])
                );
                break;
            
            case RECORD_COMMAND_BUFFER:
                recordCommandBuffer(*std::get<VkCommandBuffer*>(job->data[0]),
                    std::get<uint32_t>(job->data[1]),
                    *std::get<VkDevice*>(job->data[2]),
                    *std::get<VkPhysicalDevice*>(job->data[3])
                );
                break;

            case DUMMY_JOB:
                std::cout<<"Dummy job executed at thread: "<<id<<std::endl;
                break;

            case PREPARE_FOR_JOIN:
                cleanup(*std::get<VkDevice*>(job->data[0]),
                    commandPool
                );
                break;
            default:
                break;
        }

        Engine::threadsAvailable.add(1);

    }

    void WorkerThread::detach(){
        thread.detach();
    }

    void WorkerThread::createPoolAndBuffers(VkDevice& device, VkPhysicalDevice& physicalDevice, VkSurfaceKHR& surface){
        BufferManager::createCommandPool(physicalDevice, surface,device,commandPool);
        BufferManager::createCommandBuffers(device, commandBuffers, commandPool);
    }
}