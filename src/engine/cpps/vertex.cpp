#include "../../objects/headers/mesh.h"
#include "../headers/engine.h"

using namespace Prometheus;

namespace Prometheus{
    std::array<VkVertexInputBindingDescription,2> Vertex::getBindingDescription(){

        std::array<VkVertexInputBindingDescription,2> bindingDescriptions = {};

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        /*
            -- VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
            -- VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
        */

        bindingDescriptions[1].binding   = 1;
        bindingDescriptions[1].stride    = sizeof(InstanceInfo);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescriptions;
    }

    std::array<VkVertexInputAttributeDescription, 9> Vertex::getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 9> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        for (uint32_t i = 0; i < 4; i++) {
            attributeDescriptions[3 + i].location = 3 + i;                  // locations 3,4,5,6
            attributeDescriptions[3 + i].binding  = 1;                      // comes from instance buffer
            attributeDescriptions[3 + i].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[3 + i].offset   = offsetof(InstanceInfo, modelMatrix) + sizeof(glm::vec4) * i;
        }

        attributeDescriptions[7].binding  = 1;
        attributeDescriptions[7].location = 7; // pick next free location
        attributeDescriptions[7].format   = VK_FORMAT_R32_UINT;
        attributeDescriptions[7].offset   = offsetof(InstanceInfo, textureIndex);

        attributeDescriptions[8].binding = 0;
        attributeDescriptions[8].location = 8;
        attributeDescriptions[8].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[8].offset = offsetof(Vertex, normal);

        return attributeDescriptions;
    }

    VkVertexInputBindingDescription Vertex::getDebugBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding   = 0;
        binding.stride    = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    std::array<VkVertexInputAttributeDescription, 2> Vertex::getDebugAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attrs{};

        // Position
        attrs[0].location = 0;
        attrs[0].binding  = 0;
        attrs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[0].offset   = offsetof(Vertex, pos);

        // Color
        attrs[1].location = 1;
        attrs[1].binding  = 0;
        attrs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attrs[1].offset   = offsetof(Vertex, color);

        return attrs;
    }
}