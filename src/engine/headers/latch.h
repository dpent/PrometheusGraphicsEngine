#pragma once
#include <mutex>
#include <semaphore.h>


namespace Prometheus{


    class Latch{
        
        std::mutex mutex;
        uint64_t count;
        sem_t countDoneSemaphore;

    public:
        Latch(uint64_t countSize);

        void count_down();

        void wait();

        void setCount(uint64_t value);

        void setCountNoSync(uint64_t value);

        uint64_t getCount();

        void post();

        ~Latch();
    };
}