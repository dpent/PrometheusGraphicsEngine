#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <semaphore.h>

namespace Prometheus{
    void loadModel(std::string modelPath, sem_t& meshLoadSemaphore);

    void removeUnusedMeshes();
}