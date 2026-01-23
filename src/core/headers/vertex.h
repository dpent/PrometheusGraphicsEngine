#pragma once

#include "Prometheus.h"

struct Vertex {
public:
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
    glm::vec3 normal;
    glm::vec4 tangent{0.0f,0.0f,0.0f,0.0f};

    static std::array<VkVertexInputBindingDescription, 1> getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

struct CameraObject
{
    glm::mat4 view;
    glm::mat4 proj;
    uint32_t objectIndex;
};

struct LightVPObject {
    glm::mat4 lightVP;
    uint32_t objectIndex;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}