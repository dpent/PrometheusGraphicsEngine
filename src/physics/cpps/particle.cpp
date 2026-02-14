#include "../headers/particle.h"

Particle::Particle(){}

Particle::Particle(glm::vec3 position, glm::vec3 color, glm::vec3 velocity) {
	this->position = glm::vec4(position, 1.0f);
	this->color = glm::vec4(color, 1.0f);
	this->velocity = glm::vec4(velocity, 1.0f);
}

std::array<VkVertexInputAttributeDescription, 2> Particle::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Particle, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Particle, color);

    return attributeDescriptions;
}

std::array<VkVertexInputBindingDescription, 1> Particle::getBindingDescription() {

    std::array<VkVertexInputBindingDescription, 1> bindingDescriptions = {};

    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Particle);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return bindingDescriptions;
}