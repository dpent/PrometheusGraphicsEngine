#pragma once
#include "../../core/headers/Prometheus.h"

struct RayVertex {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 normal;

	bool operator==(const RayVertex& other) const
	{
		return position == other.position &&
			color == other.color;
	}
};

namespace std {
	template<> struct hash<RayVertex> {
		size_t operator()(RayVertex const& vertex) const {
			return ((hash<glm::vec4>()(vertex.position) ^
				(hash<glm::vec4>()(vertex.color) << 1)) >> 1);
		}
	};
}

struct RTTriangle {
	glm::ivec4 vertexIndices; // w: material index
};

struct BVHNode {
	glm::ivec4 childIndices; // x: left child, y: right child, z: first triangle index, w: triangle count
	glm::vec4 minBounds;
	glm::vec4 maxBounds;
};

class BVH {
public:

	static constexpr uint32_t MAX_TRIANGLES_PER_NODE = 12;
	static constexpr uint32_t MAX_DEPTH = 32;

	static void buildBVH(
		std::vector<RTTriangle>& triangles, 
		uint32_t triangleOffset,
		std::vector<BVHNode>& nodes,
		glm::vec3 maxCoords,
		glm::vec3 minCoords,
		std::vector<glm::vec3> centroids,
		std::vector<RayVertex>& vertices
	);
	static void splitBVHNode(
		uint32_t nodeIndex,
		std::vector<RTTriangle>& triangles,
		uint32_t triangleOffset,
		std::vector<BVHNode>& nodes,
		std::vector<glm::vec3>& centroids,
		std::vector<RayVertex>& vertices,
		uint32_t depth
	);
	static std::vector<glm::vec3> caclulateCentroids(std::vector<RTTriangle>&triangles, std::vector<RayVertex>& vertices);
	static void extendBoundaries(BVHNode& node, std::vector<RTTriangle>& triangles, std::vector<RayVertex>& vertices);
	static float getCentroidMidPoint(BVHNode& node, int axis, std::vector<glm::vec3>& centroids, glm::vec3 maxBounds);

	static void printBVH(std::vector<BVHNode>& nodes);
};

struct TLASNode {
	glm::ivec4 nodeIndices; // x: first node, y: node count
	glm::vec4 minBounds;
	glm::vec4 maxBounds;
};

class TLAS {
public:

	static void addToTLAS(std::vector<TLASNode>& tlasNodes, uint32_t firstNode, uint32_t nodeCount, glm::vec3& maxCoords, glm::vec3& minCoords);

};

struct RTMaterial {
	glm::vec4 color;
	glm::vec4 properties; // x: type
};

// For metals & lambertians, the y value is the roughness