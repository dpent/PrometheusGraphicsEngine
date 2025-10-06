#pragma once

#include "tiny_obj_loader.h"
#include <semaphore.h>


namespace Prometheus{
    class ModelManager{
    public:
        static void loadModel(std::string modelPath, sem_t& meshLoadSemaphore);
    };
}