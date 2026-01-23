#include "../headers/vertex.h"


std::array<VkVertexInputBindingDescription, 1> Vertex::getBindingDescription() {

    std::array<VkVertexInputBindingDescription, 1> bindingDescriptions = {};

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    /*
        -- VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
        -- VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
    */

    return bindingDescriptions;
}

std::array<VkVertexInputAttributeDescription, 5> Vertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

    attributeDescriptions[0].binding = 0; //POSITION
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0; //COLOR
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0; //UV COORDS
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    attributeDescriptions[3].binding = 0; //NORMAL
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(Vertex, normal);

    attributeDescriptions[4].binding = 0; //TANGENT
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(Vertex, tangent);

    return attributeDescriptions;
}