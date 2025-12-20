#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <semaphore>

namespace Prometheus{
    void loadModel(std::string modelPath, std::binary_semaphore& meshLoadSemaphore);

    void removeUnusedMeshes();
}