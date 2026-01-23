#define TINYOBJLOADER_IMPLEMENTATION
#include "../headers/mesh.h"
#include "../../core/headers/engine.h"

Mesh::Mesh() {
}

Mesh::Mesh(std::string meshPath) {
    this->meshPath = meshPath;
    this->instances = 0;
    load();
    Engine::meshMutex.lock();
	Engine::meshes.push(this);
    Engine::meshMutex.unlock();
    Engine::remakeVertexIndexBuffer = true;
}

void Mesh::load() {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
        ((std::filesystem::path(MODEL_DIR) / meshPath).lexically_normal().string().c_str()))) {
        throw std::runtime_error(err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    std::unordered_map<Vertex, glm::vec3> bitangents{};

    glm::vec3 minCoords = glm::vec3(
        attrib.vertices[shapes[0].mesh.indices[0].vertex_index],
        attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 1],
        attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 2]);

    glm::vec3 maxCoords = glm::vec3(
        attrib.vertices[shapes[0].mesh.indices[0].vertex_index],
        attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 1],
        attrib.vertices[shapes[0].mesh.indices[0].vertex_index + 2]);

    for (const auto& shape : shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {

            tinyobj::index_t idx0 = shape.mesh.indices[i];
            tinyobj::index_t idx1 = shape.mesh.indices[i + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[i + 2];

            Vertex v0 = {};
            Vertex v1 = {};
            Vertex v2 = {};

            Mesh::loadVertex(attrib, idx0, minCoords, maxCoords, v0);
            Mesh::loadVertex(attrib, idx1, minCoords, maxCoords, v1);
            Mesh::loadVertex(attrib, idx2, minCoords, maxCoords, v2);

            if (attrib.normals.empty()) {
                Mesh::computeNormals(v0, v1, v2);
            }

            if (uniqueVertices.count(v0) == 0) {
                uniqueVertices[v0] = static_cast<uint32_t>(vertices.size());
                bitangents[v0] = glm::vec3(0.0f);
                vertices.push_back(v0);
            }
            indices.push_back(uniqueVertices[v0]);
            uint32_t ind0 = uniqueVertices[v0];

            if (uniqueVertices.count(v1) == 0) {
                uniqueVertices[v1] = static_cast<uint32_t>(vertices.size());
                bitangents[v1] = glm::vec3(0.0f);
                vertices.push_back(v1);
            }
            indices.push_back(uniqueVertices[v1]);
            uint32_t ind1 = uniqueVertices[v1];

            if (uniqueVertices.count(v2) == 0) {
                uniqueVertices[v2] = static_cast<uint32_t>(vertices.size());
                bitangents[v2] = glm::vec3(0.0f);
                vertices.push_back(v2);
            }
            indices.push_back(uniqueVertices[v2]);
            uint32_t ind2 = uniqueVertices[v2];

            glm::vec3 edge1 = v1.pos - v0.pos;
            glm::vec3 edge2 = v2.pos - v0.pos;
            glm::vec2 duv1 = v1.texCoord - v0.texCoord;
            glm::vec2 duv2 = v2.texCoord - v0.texCoord;

            float denom = duv1.x * duv2.y - duv2.x * duv1.y;
            if (fabs(denom) > 1e-6f) {
                float f = 1.0f / denom;
                glm::vec4 tangent = glm::vec4(glm::vec3(f * (edge1 * duv2.y - edge2 * duv1.y)), 0.0f);
                glm::vec3 bitangent = f * (edge2 * duv1.x - edge1 * duv2.x);

                vertices[ind0].tangent += tangent;
                bitangents[v0] += bitangent;
                vertices[ind1].tangent += tangent;
                bitangents[v1] += bitangent;
                vertices[ind2].tangent += tangent;
                bitangents[v2] += bitangent;
            }

        }
    } 

    if (attrib.normals.empty()) {
        for (auto& v : vertices) {
            v.normal = glm::normalize(v.normal);
            v.tangent = glm::vec4(glm::normalize(glm::vec3(v.tangent) - v.normal * glm::dot(v.normal, glm::vec3(v.tangent))), 1.0f);

            glm::vec3 computedBitangent = glm::cross(v.normal, glm::vec3(v.tangent));
            float handedness = (glm::dot(computedBitangent, bitangents[v]) < 0.0f) ? -1.0f : 1.0f;

            v.tangent.w = handedness;
        }

        return;
    }

    for (auto& v : vertices) {
        v.tangent = glm::vec4(glm::normalize(glm::vec3(v.tangent) - v.normal * glm::dot(v.normal, glm::vec3(v.tangent))), 1.0f);

        glm::vec3 computedBitangent = glm::cross(v.normal, glm::vec3(v.tangent));
        float handedness = (glm::dot(computedBitangent, bitangents[v]) < 0.0f) ? -1.0f : 1.0f;

        v.tangent.w = handedness;
    }
}

std::string Mesh::toString() {
    std::ostringstream oss;
    oss << "Mesh { "
        << "meshPath = " << meshPath << "\n "
        << "vertexOffset = " << vertexOffset << "\n "
        << "vertices = " << vertices.size() << "\n "
        << "indexOffset = " << indexOffset << "\n "
        << "indices = " << indices.size() << "\n"
        << "instances = " << instances << " }";

    return oss.str();
}

void Mesh::loadVertex(tinyobj::attrib_t& attrib, tinyobj::index_t& index, glm::vec3& minCoords, glm::vec3& maxCoords, Vertex& vertex) {

    vertex.pos = {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
    };

    if (!attrib.normals.empty()) {

        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2]
        };
    }
    else {

        vertex.normal = glm::vec3(0.0f);
    }

    if (vertex.pos.x < minCoords.x) minCoords.x = vertex.pos.x;
    if (vertex.pos.y < minCoords.y) minCoords.y = vertex.pos.y;
    if (vertex.pos.z < minCoords.z) minCoords.z = vertex.pos.z;

    if (vertex.pos.x > maxCoords.x) maxCoords.x = vertex.pos.x;
    if (vertex.pos.y > maxCoords.y) maxCoords.y = vertex.pos.y;
    if (vertex.pos.z > maxCoords.z) maxCoords.z = vertex.pos.z;

    if (
        !attrib.texcoords.empty() &&
        index.texcoord_index >= 0 &&
        static_cast<size_t>(2 * index.texcoord_index + 1) < attrib.texcoords.size()
        ) {
        vertex.texCoord = {
        attrib.texcoords[2 * index.texcoord_index + 0],
        1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
        };
    }
    else {
        vertex.texCoord = { 0.0f, 0.0f };
    }

    vertex.color = { 1.0f, 1.0f, 1.0f }; //This means that the model will colored by the texture color
}

void Mesh::computeNormals(Vertex& v0, Vertex& v1, Vertex& v2) {

    glm::vec3 edge1 = v1.pos - v0.pos;
    glm::vec3 edge2 = v2.pos - v0.pos;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    v0.normal += faceNormal;
    v1.normal += faceNormal;
    v2.normal += faceNormal;

}