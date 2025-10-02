#pragma once

#include <array>
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace Prometheus{
    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static std::array<VkVertexInputBindingDescription,2> getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 8> getAttributeDescriptions();

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    struct CameraObject
    {
        glm::mat4 view;
        glm::mat4 proj;   
    };
    

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };
}

namespace std {
    template<> struct hash<Prometheus::Vertex> {
        size_t operator()(Prometheus::Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}