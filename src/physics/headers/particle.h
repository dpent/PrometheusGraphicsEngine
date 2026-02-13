#pragma once

#include "../../core/headers/Prometheus.h"

struct Particle {
public:

	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 velocity;

	Particle();
	Particle(glm::vec3 position, glm::vec3 color, glm::vec3 velocity);

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	static std::array<VkVertexInputBindingDescription, 1> getBindingDescription();

	bool operator==(const Particle& other) const {
		return position == other.position && color == other.color && velocity == other.velocity;
	}
};