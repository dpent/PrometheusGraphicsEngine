#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <semaphore>
#include <algorithm>
#include "../../engine/headers/modelManager.h"
#include "../../engine/headers/descriptorManager.h"
#include <unordered_map>
#include "../../objects/headers/gameObject.h"
#include "../../objects/headers/mesh.h"
#include "../../engine/headers/latch.h"


namespace Prometheus
{
    void createObject(GameObject* obj,
        VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue,
        VkCommandPool& commandPool);

    void deleteObject(GameObject* object,VkDevice& device);

    void updateTextureDeleteQueue(VkDevice& device);

    void updateGameObjects(Latch* latch, std::binary_semaphore* setReady);

    void updateObjectsAndDescriptors(VkDevice& device, std::counting_semaphore<INT_MAX>* jobDoneSem, std::binary_semaphore* safeToMakeInstanceBuffer,
        Latch* latch, std::binary_semaphore* setReady);
    void splitObjectsAndCreateJobs(uint64_t& objectsPerThread,
        std::vector<std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>>& objectPieces,
        Latch& latch, std::vector<std::unordered_map<std::string,MeshBatch>>& batchPieces);
}
