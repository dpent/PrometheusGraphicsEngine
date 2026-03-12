#pragma once

#include "../../core/headers/Prometheus.h"
#include "../../core/headers/vertex.h"

struct RayVertex;
struct RTTriangle;

class Mesh {
public:
    uint32_t vertexOffset;
    uint32_t indexOffset;
    std::string meshPath;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t instances;

    Mesh* next = nullptr;
    Mesh* prev = nullptr;

    Mesh();
    Mesh(std::string meshPath);
    std::string toString();

    void load();
    static void loadVertex(tinyobj::attrib_t& attrib, tinyobj::index_t& index, glm::vec3& minCoords, glm::vec3& maxCoords, Vertex& vertex);
    static void computeNormals(Vertex& v0, Vertex& v1, Vertex& v2);

    static void loadForRayTrace(
        std::vector<RayVertex>& vertices,
        std::vector<RTTriangle>& triangles, 
        std::string meshPath, 
        glm::vec3 scale,
        glm::vec3 translation, 
        glm::vec3 rotation,
		uint32_t materialIndex
    );

    static void loadRayVertex(
        tinyobj::attrib_t& attrib, 
        tinyobj::index_t& index,
        glm::vec3& minCoords, 
        glm::vec3& maxCoords, 
        RayVertex& vertex, 
        glm::vec3& scale, 
        glm::vec3& translation, 
        glm::quat& rotationQ
    );

    static void computeRayNormals(
        RayVertex& v0, 
        RayVertex& v1, 
        RayVertex& v2
    );
};