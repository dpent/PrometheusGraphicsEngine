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
#include "instanceInfo.h"


namespace Prometheus{

    class GameObject;

    struct Mesh{
        uint32_t vertexOffset;
        uint32_t indexOffset;
        std::string meshPath;

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::array<glm::vec3, 8> hitboxPoints;
        glm::vec3 center;
        /*
        0: Top front left
        1: Top front right
        2: Top back left
        3: Top back right
        4: Bottom front left
        5: Bottom front right
        6: Bottom back left
        7: Bottom back right
        */

        Mesh();
        Mesh(std::string meshPath, std::vector<Vertex> vertices, std::vector<uint32_t> indices,
            glm::vec3 min, glm::vec3 max);
        std::string toString();

        void drawAABB();
    };

    struct MeshBatch{
        std::string meshPath;
        std::vector<InstanceInfo> instances;
        std::vector<GameObject*> objects;
        std::vector<Texture*> textures;

        MeshBatch(std::string path);
        MeshBatch();
    };
}