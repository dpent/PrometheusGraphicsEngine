#include "../headers/threadManager.h"

using namespace Prometheus;

namespace Prometheus{
    
    ThreadManager::ThreadManager(){

        sem_init(&workInQueueSemaphore,0,1);
    }

    void ThreadManager::start(uint16_t poolSize){
        std::cout<<"\nThreads controlled by the thread manager: "<<poolSize<<"\n"<<std::endl;
        for (uint16_t i=0; i<poolSize; i++){
            WorkerThread* wt = new WorkerThread();

            threadPool[wt->id]=wt;
        }

        queueMutex.lock();

        managerThread=std::thread(&ThreadManager::managerLoop, this);

        queueMutex.unlock();

    }

    void ThreadManager::managerLoop(){
        
        while(alive){

            queueMutex.lock();

            if(!jobQueue.empty()){

                std::vector<std::queue<Job*>> batches = batchJobs();

                queueMutex.unlock();

                std::srand(std::time(nullptr));

                for(size_t i = 0; i<batches.size(); i++){
                    int randomIndex = std::rand() % threadPool.size();

                    auto it = threadPool.begin();
                    std::advance(it, randomIndex);

                    WorkerThread* wt = it->second;

                    wt->jobsMutex.lock();
                    while(!batches[i].empty()){
                        wt->jobs.push(batches[i].front());
                        batches[i].pop();
                    }
                    wt->jobsMutex.unlock();

                    int sval;
                    sem_getvalue(&wt->workSemaphore, &sval);
                    if (sval == 0) {
                        sem_post(&wt->workSemaphore);
                    }
                }
            }else{
                queueMutex.unlock();
                sem_wait(&workInQueueSemaphore);
            }

            queueMutex.unlock();
        }

        for (auto& pair : threadPool) {
            if (pair.second->thread.joinable()) {
                pair.second->thread.join();
            }
            delete pair.second;
        }
        threadPool.clear();
    }

    void ThreadManager::detach(){
        managerThread.detach();
    }

    std::vector<std::queue<Job*>> ThreadManager::batchJobs(){

        std::vector<std::queue<Job*>> batches;
        std::unordered_map<std::string,std::queue<Job*>> batchMap;

        while(!jobQueue.empty()){
            Job* j = new Job(jobQueue.front());
            //std::cout<<j->opId<<std::endl;

            switch (j->opId){
                
                case 0:
                case 1:
                case 2:
                case 3:

                    batchMap["0123"].push(j);
                    break;
                
                case 4:
                    batchMap["4"].push(j);
                    break;
                
                case 5:

                    batchMap["5"].push(j);
                    break;

                case 6:
                    batchMap["6"].push(j);
                    break;

            }
            jobQueue.pop();
        }

        batches.reserve(batchMap.size());

        for (auto& pair : batchMap) {
            batches.push_back(pair.second);
        }

        return batches;
    }

    void ThreadManager::terminate(){
        for (const auto& pair : threadPool) {
            pair.second->alive=false;
            sem_post(&(pair.second->workSemaphore));
        }

        alive=false;
    }
}