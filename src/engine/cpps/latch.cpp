#include "../headers/latch.h"
#include <semaphore>

using namespace Prometheus;


namespace Prometheus{
 
    Latch::Latch(uint64_t countSize){
        this->count=countSize;
    }

    void Latch::count_down(){

        mutex.lock();

        count--;

        if(count==0){
            this->countDoneSemaphore.release();
        }

        mutex.unlock();
    }

    void Latch::post(){
        this->countDoneSemaphore.release();
    }

    void Latch::wait(){

        this->countDoneSemaphore.acquire();
    }

    Latch::~Latch(){
    }

    void Latch::setCount(uint64_t value){
        mutex.lock();

        count = value;

        if(count==0){
            this->countDoneSemaphore.release();
        }

        mutex.unlock();
    }

    void Latch::setCountNoSync(uint64_t value){
        count = value;

        if(count==0){
            this->countDoneSemaphore.release();
        }
    }

    uint64_t Latch::getCount(){
        return count;
    }
}
