#include "../headers/vertex.h"
#include "../../objects/headers/mesh.h"

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

    std::array<VkVertexInputAttributeDescription, 7> Vertex::getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};

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

        return attributeDescriptions;
    }
}