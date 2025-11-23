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

        static VkVertexInputBindingDescription getDebugBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 2> getDebugAttributeDescriptions();

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    struct CameraObject
    {
        glm::mat4 view;
        glm::mat4 proj; 
    };

    struct GridInfoObject
    {
        glm::mat4 view;
        glm::mat4 projection;   
        glm::vec3 cameraPos;
        float gridSize;
    };
    

    struct UniformBufferObject {
        public:

        glm::vec4 position;
        glm::vec4 color;
        glm::vec4 ambientLightColor;
        float intensity;

        UniformBufferObject(glm::vec4 pos, glm::vec4 color, float intensity);
        UniformBufferObject();
    };

    struct UBOData {
        glm::vec4 positions[128];
        glm::vec4 colors[128];
        glm::vec4 ambientLightColors[128];
        alignas(16) float intensities[128];
        uint64_t lightCount;
    };

    class UBOContainer {
        public:
        UniformBufferObject* ubo;
        UBOContainer* next;
        UBOContainer* prev;

        UBOContainer(UniformBufferObject* ubo);
        UBOContainer();
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