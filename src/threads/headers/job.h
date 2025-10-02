#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <variant>
#include <string>


namespace Prometheus{
    enum operationId : uint16_t{
        CREATE_OBJECT = 0,
        DELETE_OBJECT = 1,
        MAKE_VERTEX_INDEX_BUFFER = 2,
        MAKE_INSTANCE_BUFFER = 3,
        MAKE_COMMAND_BUFFER = 4,
        LOAD_MODEL = 5,
        APPLY_INPUT = 6
    };

    struct Job{
    public:
        operationId opId;
        uint64_t threadId;
        std::vector<std::variant<std::string,
        VkDevice*,VkPhysicalDevice*,VkQueue*, uint64_t>> data;

        Job(operationId opId);
    };
}