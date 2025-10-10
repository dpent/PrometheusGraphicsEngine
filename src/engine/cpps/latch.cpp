#include "../headers/latch.h"
#include <semaphore.h>

using namespace Prometheus;


namespace Prometheus{
 
    Latch::Latch(uint64_t countSize){
        this->count=countSize;
        sem_init(&(this->countDoneSemaphore),0,0);
    }

    void Latch::count_down(){

        mutex.lock();

        count--;

        if(count==0){
            sem_post(&(this->countDoneSemaphore));
        }

        mutex.unlock();
    }

    void Latch::wait(){

        sem_wait(&(this->countDoneSemaphore));
    }

    Latch::~Latch(){
        sem_destroy(&(this->countDoneSemaphore));
    }
}
