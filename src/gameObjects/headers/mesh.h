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

    Mesh();
    Mesh(std::string meshPath);
    std::string toString();

    void load();
};