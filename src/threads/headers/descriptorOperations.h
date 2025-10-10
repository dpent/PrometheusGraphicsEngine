#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <semaphore.h>
#include "../../engine/headers/descriptorManager.h"

namespace Prometheus{
    void updateDescriptorDeleteQueue(VkDevice& device);

    void recreateDescriptors(VkDevice& device, sem_t* jobDoneSem);
}