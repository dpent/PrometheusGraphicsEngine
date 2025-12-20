#pragma once

#include "tiny_obj_loader.h"
#include <semaphore>


namespace Prometheus{
    class ModelManager{
    public:
        static void loadModel(std::string modelPath, std::binary_semaphore& meshLoadSemaphore);
    };
}