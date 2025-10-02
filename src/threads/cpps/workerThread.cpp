#include "../headers/workerThread.h"
#include "../headers/gameObjectOperations.h"


using namespace Prometheus;

namespace Prometheus{

    WorkerThread::WorkerThread(){

        sem_init(&(this->workSemaphore),0,1);
        this->thread = std::thread(&WorkerThread::workerLoop, this);
        id=thread.get_id();
    }

    void WorkerThread::workerLoop(){

        while(alive){

            if(!(jobs.empty())){
                jobsMutex.lock();

                Job* job = jobs.front();
                jobs.pop();

                jobsMutex.unlock();
                
                doWork(job);
            }else if(alive){
                sem_wait(&(this->workSemaphore));
            }

        }

        while(!jobs.empty()){
            jobsMutex.lock();

            delete jobs.front();
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
            default:
                break;
        }

        delete job;
    }

    void WorkerThread::detach(){
        thread.detach();
    }
}