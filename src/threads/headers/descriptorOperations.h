#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <semaphore>
#include "../../engine/headers/descriptorManager.h"

namespace Prometheus{
    void updateDescriptorDeleteQueue(VkDevice& device);

    void recreateDescriptorSetsAndPool(VkDevice& device, std::binary_semaphore* jobDoneSem);
}