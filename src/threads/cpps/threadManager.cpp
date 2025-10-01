#include "../headers/threadManager.h"

using namespace Prometheus;

namespace Prometheus{
    
    ThreadManager::ThreadManager(){
    }

    void ThreadManager::start(uint16_t poolSize){

        poolMutex.lock();

        for (uint16_t i=0; i<poolSize; i++){
            WorkerThread* wt = new WorkerThread(&poolMutex, &threadPool, &activeThreads);

            threadPool[wt->id]=wt;

            wt->detach();
        }

        poolMutex.unlock();

        managerThread=std::thread(&ThreadManager::managerLoop, this);
    }

    void ThreadManager::managerLoop(){
        
        while(true){
            if(!jobQueue.empty()){

                if(threadPool.size()==0){
                    continue;
                }

                std::srand(std::time(nullptr));

                int randomIndex = std::rand() % threadPool.size();

                auto it = threadPool.begin();
                std::advance(it, randomIndex);

                WorkerThread* wt = it->second;

                queueMutex.lock();

                wt->jobs.push(&(jobQueue.front()));
                sem_post(&(wt->workSemaphore));
                jobQueue.pop();

                queueMutex.unlock();

                poolMutex.lock();

                activeThreads[it->first]=it->second;
                threadPool.erase(it->first);

                poolMutex.unlock();

            }else{
                jobQueue.emplace(Job(CREATE_OBJECT));
            }
        }
    }

    void ThreadManager::detach(){
        managerThread.detach();
    }
}