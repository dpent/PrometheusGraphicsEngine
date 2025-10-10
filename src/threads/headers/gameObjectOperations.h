#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <semaphore.h>
#include <algorithm>
#include "../../engine/headers/modelManager.h"
#include "../../engine/headers/descriptorManager.h"
#include <unordered_map>
#include "../../objects/headers/gameObject.h"
#include "../../objects/headers/mesh.h"
#include "../../engine/headers/latch.h"


namespace Prometheus
{
    void createObject(std::string texturePath, std::string modelPath, 
        VkDevice& device, VkPhysicalDevice& physicalDevice, VkQueue& graphicsQueue);

    void deleteObject(uint64_t id,VkDevice& device);

    void updateTextureDeleteQueue(VkDevice& device);

    void updateGameObjects(std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>& objectsByMesh,
        std::unordered_map<std::string,MeshBatch>& batchBuffer, Latch& latch);

    void updateObjectsAndDescriptors(VkDevice& device, sem_t* jobDoneSem, sem_t* safeToMakeInstanceBuffer);
    void splitObjectsAndCreateJobs(uint64_t& objectsPerThread,
        std::vector<std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>>& objectPieces,
        Latch& latch, std::vector<std::unordered_map<std::string,MeshBatch>>& batchPieces);
    void mergeAllThreadBatches(std::vector<std::unordered_map<std::string,MeshBatch>>& batchPieces);
}
