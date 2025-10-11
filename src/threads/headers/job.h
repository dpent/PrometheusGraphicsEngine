#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <variant>
#include <string>
#include <semaphore.h>
#include <unordered_map>
#include "../../objects/headers/gameObject.h"
#include "../../objects/headers/mesh.h"
#include "../../engine/headers/latch.h"

namespace Prometheus{
    enum operationId : uint16_t{
        CREATE_OBJECT = 0,
        DELETE_OBJECT = 1,
        UPDATE_VERTEX_INDEX_BUFFER = 2,
        UPDATE_INSTANCE_BUFFER = 3,
        MAKE_INSTANCE_BUFFER = 4,
        MAKE_COMMAND_BUFFER = 5,
        LOAD_MODEL = 6,
        APPLY_INPUT = 7,
        UPDATE_TEXTURE_DELETE_QUEUE = 8,
        UPDATE_MESH_DATA_STRUCTURES = 9,
        UPDATE_DESCRIPTOR_DELETE_QUEUE = 10,
        RECREATE_DESCRIPTORS = 11,
        UPDATE_GAME_OBJECTS = 12,
        UPDATE_OBJECTS_AND_DESCRIPTORS = 13,
        RECORD_COMMAND_BUFFER = 14
    };

    struct Job{
    public:
        operationId opId;
        uint64_t threadId;

        std::vector<std::variant<std::string,
        VkDevice*,
        VkPhysicalDevice*,
        VkQueue*, 
        uint64_t,
        sem_t*,
        std::unordered_map<std::string,std::unordered_map<uint64_t,GameObject*>>*,
        std::unordered_map<std::string,MeshBatch>*, 
        Latch*>> data;

        Job(operationId opId);
    };
}