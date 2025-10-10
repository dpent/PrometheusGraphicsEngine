#include "../headers/workerThread.h"
#include "../headers/gameObjectOperations.h"
#include "../headers/descriptorOperations.h"
#include "../headers/modelOperations.h"
#include "../headers/bufferOperations.h"
#include "../../engine/headers/engine.h"


using namespace Prometheus;

namespace Prometheus{

    WorkerThread::WorkerThread(){
        this->thread = std::thread(&WorkerThread::workerLoop, this);
        id=thread.get_id();
    }

    void WorkerThread::workerLoop(){

        while(alive){

            if(!(jobs.empty())){
                jobsMutex.lock();

                Job job = jobs.front();
                jobs.pop();

                jobsMutex.unlock();
                
                doWork(&job);
            }else if(alive){
                sem_wait(&(Engine::workInQueueSemaphore));

                jobsMutex.lock();
                Engine::queueMutex.lock();

                Engine::threadsAvailable.add(-1);

                jobs.push(Engine::jobQueue.front());
                
                //std::cout<<"Got a "<<jobs.front().opId<<" job."<<std::endl;
                Engine::jobQueue.pop();
                //std::cout<<Engine::jobQueue.size()<<" More jobs in queue."<<std::endl;

                Engine::queueMutex.unlock();
                jobsMutex.unlock();
            }

        }

        while(!jobs.empty()){

            jobsMutex.lock();
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
                    *std::get<VkQueue*>(job->data[4])
                );
                break;
            
            case DELETE_OBJECT:
                deleteObject(std::get<uint64_t>(job->data[0]),
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
                    *std::get<std::unordered_map<std::string, 
                            std::unordered_map<uint64_t, Prometheus::GameObject *>>*>(job->data[0]),
                    *std::get<std::unordered_map<std::string, Prometheus::MeshBatch>*>(job->data[1]),
                    *std::get<Latch*>(job->data[2])
                );
                break;
            
            case UPDATE_OBJECTS_AND_DESCRIPTORS:
                updateObjectsAndDescriptors(*std::get<VkDevice*>(job->data[0]),
                    std::get<sem_t*>(job->data[1]),
                    std::get<sem_t*>(job->data[2])
                );
                break;

            case UPDATE_VERTEX_INDEX_BUFFER:
                updateVertexIndexBuffer(*std::get<VkDevice*>(job->data[0]),
                    *std::get<VkPhysicalDevice*>(job->data[1]), 
                    *std::get<VkQueue*>(job->data[2]),
                    std::get<sem_t*>(job->data[3])
                );
                break;

            case UPDATE_INSTANCE_BUFFER:
                break;
                
            case MAKE_INSTANCE_BUFFER:
                recreateInstanceBuffers(*std::get<VkDevice*>(job->data[0]),
                    *std::get<VkPhysicalDevice*>(job->data[1]),
                    std::get<sem_t*>(job->data[2])
                );
                break;

            default:
                break;
        }

        Engine::threadsAvailable.add(1);

        //std::cout<<"=== Completed the job === "<<"Job code was "<<job->opId<<std::endl;
    }

    void WorkerThread::detach(){
        thread.detach();
    }
}