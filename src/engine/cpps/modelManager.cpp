#define TINYOBJLOADER_IMPLEMENTATION
#include "../headers/modelManager.h"
#include <vulkan/vulkan_core.h>
#include "../headers/engine.h"
#include <unordered_map>

using namespace Prometheus;

namespace Prometheus{
    void ModelManager::loadModel(std::string modelPath, std::binary_semaphore& meshLoadSemaphore){
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

		//std::cout << "Loading model: " << modelPath << "..." << std::endl;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, 
            (Engine::exeDir / modelPath).lexically_normal().string().c_str())) {
            throw std::runtime_error(err);
        }

        //std::cout << "DONE" << std::endl;

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

                if(!attrib.normals.empty()){

                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }else{

                    vertex.normal = glm::vec3(0.0f);
                }

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

                vertex.color = {1.0f, 1.0f, 1.0f}; //This means that the model will colored by the texture color

                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }

                indices.push_back(uniqueVertices[vertex]);
            }
        }

        if(attrib.normals.empty()){
            for (const auto& shape : shapes) { 
                for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {
                    
                    auto idx0 = shape.mesh.indices[i + 0].vertex_index;
                    auto idx1 = shape.mesh.indices[i + 1].vertex_index;
                    auto idx2 = shape.mesh.indices[i + 2].vertex_index;

                    Vertex v0, v1, v2;

                    v0.pos = {
                        attrib.vertices[3 * idx0 + 0],
                        attrib.vertices[3 * idx0 + 1],
                        attrib.vertices[3 * idx0 + 2]
                    };
                    v1.pos = {
                        attrib.vertices[3 * idx1 + 0],
                        attrib.vertices[3 * idx1 + 1],
                        attrib.vertices[3 * idx1 + 2]
                    };
                    v2.pos = {
                        attrib.vertices[3 * idx2 + 0],
                        attrib.vertices[3 * idx2 + 1],
                        attrib.vertices[3 * idx2 + 2]
                    };

                    glm::vec3 edge1 = v1.pos - v0.pos;
                    glm::vec3 edge2 = v2.pos - v0.pos;
                    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

                    vertices[idx0].normal += faceNormal;
                    vertices[idx1].normal += faceNormal;
                    vertices[idx2].normal += faceNormal;
                }
            }

            for (auto& v : vertices) {
                v.normal = glm::normalize(v.normal);
            }
        }

        Engine::meshMap[modelPath]=Mesh(modelPath,vertices,indices, minCoords, maxCoords);
        Engine::meshesLoading.erase(modelPath);

        meshLoadSemaphore.release();
    }
}