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

struct CameraVPObject {
    glm::mat4 view;
    glm::mat4 proj;
};

struct RayTracingCameraObject {
    glm::vec4 up;
    glm::vec4 forward;
    glm::vec4 right;
    glm::vec4 position;
    glm::vec4 utils;
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

struct DebugVertex {
public:
    float t; //Either 0 or 1. The vertex shader interpolates between

    static std::array<VkVertexInputBindingDescription, 1> getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions();

    bool operator==(const DebugVertex& other) const {
        return t == other.t;
    }
};

struct Line {
    glm::vec4 start;
    glm::vec4 end;
    glm::vec4 color;

    Line();
    Line(glm::vec3 start, glm::vec3 end, glm::vec3 color);
};

namespace std {
    template<> struct hash<DebugVertex> {
        size_t operator()(DebugVertex const& vertex) const {
            return (hash<float>()(vertex.t));
        }
    };
}

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}