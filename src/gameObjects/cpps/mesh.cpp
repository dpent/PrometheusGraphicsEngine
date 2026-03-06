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

void Mesh::loadForRayTrace(std::vector<RayVertex>& vertices, std::vector<RTTriangle>& triangles, 
    std::string meshPath, glm::vec3 scale, glm::vec3 translation, glm::vec3 rotation, 
    uint32_t materialIndex, glm::vec3& maxCoords, glm::vec3& minCoords
) {

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
        ((std::filesystem::path(MODEL_DIR) / meshPath).lexically_normal().string().c_str()))) {
        throw std::runtime_error(err);
    }

    std::unordered_map<RayVertex, uint32_t> uniqueVertices{};

    glm::vec3 rotationRadians = glm::radians(rotation);
    glm::quat q = glm::quat(rotationRadians);

    for (const auto& shape : shapes) {
        for (size_t i = 0; i < shape.mesh.indices.size(); i += 3) {

            tinyobj::index_t idx0 = shape.mesh.indices[i];
            tinyobj::index_t idx1 = shape.mesh.indices[i + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[i + 2];

            RayVertex v0 = {};
            RayVertex v1 = {};
            RayVertex v2 = {};

            Mesh::loadRayVertex(attrib, idx0, minCoords, maxCoords, v0, scale, translation, q);
            Mesh::loadRayVertex(attrib, idx1, minCoords, maxCoords, v1, scale, translation, q);
            Mesh::loadRayVertex(attrib, idx2, minCoords, maxCoords, v2, scale, translation, q);

            if (uniqueVertices.count(v0) == 0) {
                uniqueVertices[v0] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(v0);
            }

            if (uniqueVertices.count(v1) == 0) {
                uniqueVertices[v1] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(v1);
            }

            if (uniqueVertices.count(v2) == 0) {
                uniqueVertices[v2] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(v2);
            }

            if (attrib.normals.empty()) {
                Mesh::computeRayNormals(v0, v1, v2);
            }

            RTTriangle triangle = {
                .vertexIndices = glm::ivec4(uniqueVertices[v0], uniqueVertices[v1], uniqueVertices[v2], materialIndex)
            };

			triangles.push_back(triangle);
        }
    }

    for (size_t i = 0; i < vertices.size(); i++) {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }
}

void Mesh::loadRayVertex(tinyobj::attrib_t& attrib, tinyobj::index_t& index, glm::vec3& minCoords, glm::vec3& maxCoords, RayVertex& vertex, glm::vec3& scale, glm::vec3& translation, glm::quat& rotationQ) {

    glm::vec3 pos = {
    attrib.vertices[3 * index.vertex_index + 0],
    attrib.vertices[3 * index.vertex_index + 1],
    attrib.vertices[3 * index.vertex_index + 2]
    };

    pos *= scale;
    pos = rotationQ * pos;
    pos += translation;

    vertex.position = {
        pos,
        1.0f
    };

    if (!attrib.normals.empty()) {

        glm::vec3 n = {
        attrib.normals[3 * index.normal_index + 0],
        attrib.normals[3 * index.normal_index + 1],
        attrib.normals[3 * index.normal_index + 2]
        };
        vertex.normal = glm::vec4(rotationQ * n, 0.0f);
    }
    else {

        vertex.normal = glm::vec4(0.0f);
    }

    if (vertex.position.x < minCoords.x) minCoords.x = vertex.position.x;
    if (vertex.position.y < minCoords.y) minCoords.y = vertex.position.y;
    if (vertex.position.z < minCoords.z) minCoords.z = vertex.position.z;

    if (vertex.position.x > maxCoords.x) maxCoords.x = vertex.position.x;
    if (vertex.position.y > maxCoords.y) maxCoords.y = vertex.position.y;
    if (vertex.position.z > maxCoords.z) maxCoords.z = vertex.position.z;

    if (attrib.colors.empty()) {

        vertex.color = { 1.0f, 1.0f, 1.0f, 1.0f };
        return;

    }

    vertex.color = {
        attrib.colors[3 * index.vertex_index + 0],
        attrib.colors[3 * index.vertex_index + 1],
        attrib.colors[3 * index.vertex_index + 2],
        1.0f
    };
}

void Mesh::computeRayNormals(RayVertex& v0, RayVertex& v1, RayVertex& v2) {

    glm::vec3 edge1 = glm::vec3(v1.position) - glm::vec3(v0.position);
    glm::vec3 edge2 = glm::vec3(v2.position) - glm::vec3(v0.position);
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    v0.normal += glm::vec4(faceNormal, 0.0f);
    v1.normal += glm::vec4(faceNormal, 0.0f);
    v2.normal += glm::vec4(faceNormal, 0.0f);
}