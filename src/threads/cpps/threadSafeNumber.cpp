#include "../headers/threadSafeNumber.h"


using namespace Prometheus;

namespace Prometheus{

    SafeUint16_t::SafeUint16_t(uint16_t startValue){
        this->number=startValue;
    }

    SafeUint16_t::SafeUint16_t(){}

    void SafeUint16_t::add(uint16_t value){

        std::lock_guard<std::mutex> lock(mutex);

        number+=value;
    }

    uint16_t SafeUint16_t::getValue(){

        std::lock_guard<std::mutex> lock(mutex);

        return number;
    }
}