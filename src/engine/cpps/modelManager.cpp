#define TINYOBJLOADER_IMPLEMENTATION
#include "../headers/modelManager.h"
#include <vulkan/vulkan_core.h>
#include "../headers/engine.h"
#include <unordered_map>

using namespace Prometheus;

namespace Prometheus{
    void ModelManager::loadModel(std::string modelPath){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
            throw std::runtime_error(err);
        }

        uint32_t vertexOffset=Engine::vertices.size();
        uint32_t vertexCount=0;
        uint32_t indexOffset=Engine::indices.size();
        uint32_t indexCount=0;

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

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
                    uniqueVertices[vertex] = static_cast<uint32_t>(Engine::vertices.size() - vertexOffset);
                    Engine::vertices.push_back(vertex);
                    vertexCount++;
                }

                Engine::indices.push_back(uniqueVertices[vertex]);
                indexCount++;
            }
        }

        Engine::meshMap[modelPath]=Mesh(vertexOffset,vertexCount,indexOffset,indexCount,modelPath);
    }
}