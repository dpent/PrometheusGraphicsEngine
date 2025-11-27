#pragma once
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <array>
#include <GLFW/glfw3.h>

namespace Prometheus
{
    struct Particle{
        public:
        glm::vec3 position;

        Particle();
        Particle(glm::vec3 position);

        static void addDemoParticles(uint64_t number);

        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();
        static std::array<VkVertexInputBindingDescription,1> getBindingDescriptions();
    };
}
