#define TINYOBJLOADER_IMPLEMENTATION
#include "../headers/modelManager.h"
#include <vulkan/vulkan_core.h>
#include "../headers/engine.h"
#include <unordered_map>

using namespace Prometheus;

namespace Prometheus{
    void ModelManager::loadModel(std::string modelPath, sem_t& meshLoadSemaphore){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, 
            (Engine::exeDir / modelPath).lexically_normal().c_str())) {
            throw std::runtime_error(err);
        }

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        glm::vec3 minCoords = glm::vec3(
            attrib.vertices[shapes[0].mesh.indices[0].vertex_index],
            attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 1],
            attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 2]);

        glm::vec3 maxCoords = glm::vec3(
            attrib.vertices[shapes[0].mesh.indices[0].vertex_index],
            attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 1],
            attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 2]);

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (vertex.pos.x < minCoords.x) minCoords.x = vertex.pos.x;
                if (vertex.pos.y < minCoords.y) minCoords.y = vertex.pos.y;
                if (vertex.pos.z < minCoords.z) minCoords.z = vertex.pos.z;

                if (vertex.pos.x > maxCoords.x) maxCoords.x = vertex.pos.x;
                if (vertex.pos.y > maxCoords.y) maxCoords.y = vertex.pos.y;
                if (vertex.pos.z > maxCoords.z) maxCoords.z = vertex.pos.z;

                if(
                    !attrib.texcoords.empty() && 
                    index.texcoord_index >= 0 && 
                    static_cast<size_t>(2 * index.texcoord_index + 1) < attrib.texcoords.size()
                ){
                    vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }else{
                    vertex.texCoord = {0.0f, 0.0f};
                }

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }
        Engine::meshMap[modelPath]=Mesh(modelPath,vertices,indices, minCoords, maxCoords);
        Engine::meshesLoading.erase(modelPath);

        sem_post(&meshLoadSemaphore);
    }
}