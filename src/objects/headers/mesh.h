#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include "../../engine/headers/textureManager.h"


namespace Prometheus{
    struct Mesh{
        uint32_t vertexOffset;
        uint32_t indexOffset;
        std::string meshPath;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        Mesh();
        Mesh(std::string meshPath, std::vector<Vertex> vertices, std::vector<uint32_t> indices);
        std::string toString();
    };

    struct InstanceInfo{
        glm::mat4 modelMatrix;
        alignas(16) uint32_t textureIndex;

        InstanceInfo(glm::mat4 model, uint32_t textureIndex);

        std::string toString();
    };

    struct MeshBatch{
        std::string meshPath;
        std::vector<InstanceInfo> instances;
        std::vector<uint64_t> ids;
        std::vector<Texture> textures;

        MeshBatch(std::string path);
    };
}