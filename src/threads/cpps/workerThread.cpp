#include "../headers/workerThread.h"


using namespace Prometheus;

namespace Prometheus{

    WorkerThread::WorkerThread(std::mutex* poolMutex,std::unordered_map<std::thread::id, WorkerThread*>* threadPool,
        std::unordered_map<std::thread::id, WorkerThread*>* activeThreads
    ){

        sem_init(&(this->workSemaphore),0,1);
        this->poolMutex=poolMutex;
        this->threadPool=threadPool;
        this->activeThreads=activeThreads;
        this->thread = std::thread(&WorkerThread::workerLoop, this);
        id=thread.get_id();
    }

    void WorkerThread::workerLoop(){

        while(true){

            if(!(jobs.empty())){
                Job* job = jobs.front();
                jobs.pop();
                
                doWork(job);
            }else if(alive){
                sem_wait(&(this->workSemaphore));
            }

            if(!alive){
                break;
            }

        }
    }

    void WorkerThread::doWork(Job* job){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"Thread id: "<<id<<std::endl;

        poolMutex->lock();

        (*threadPool)[id]=this;
        activeThreads->erase(id);

        poolMutex->unlock();
    }

    void WorkerThread::detach(){
        thread.detach();
    }
}