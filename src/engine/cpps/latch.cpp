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

    void Latch::post(){
        sem_post(&(this->countDoneSemaphore));
    }

    void Latch::wait(){

        sem_wait(&(this->countDoneSemaphore));
    }

    Latch::~Latch(){
        sem_destroy(&(this->countDoneSemaphore));
    }

    void Latch::setCount(uint64_t value){
        mutex.lock();

        count = value;

        if(count==0){
            sem_post(&(this->countDoneSemaphore));
        }

        mutex.unlock();
    }

    void Latch::setCountNoSync(uint64_t value){
        count = value;

        if(count==0){
            sem_post(&(this->countDoneSemaphore));
        }
    }

    uint64_t Latch::getCount(){
        return count;
    }
}
