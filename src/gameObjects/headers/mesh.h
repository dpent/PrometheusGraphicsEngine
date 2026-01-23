#pragma once

#include "../../core/headers/Prometheus.h"
#include "../../core/headers/vertex.h"

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
};