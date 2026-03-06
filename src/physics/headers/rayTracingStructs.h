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

	static constexpr uint32_t MAX_TRIANGLES_PER_NODE = 6;
	static constexpr uint32_t MAX_DEPTH = 32;

	static void createBVH(std::vector<RTTriangle>& triangles, std::vector<BVHNode>& nodes, glm::vec3& maxCoords, glm::vec3& minCoords, std::vector<glm::vec3> centroids, std::vector<RayVertex>& vertices);
	static void splitNode(BVHNode& node, std::vector<RTTriangle>& triangles, std::vector<BVHNode>& nodes, std::vector<glm::vec3>& centroids, uint32_t depth, std::vector<RayVertex>& vertices);
	static std::vector<glm::vec3> caclulateCentroids(std::vector<RTTriangle>&triangles, std::vector<RayVertex>& vertices);
	static void extendBoundaries(BVHNode& node, std::vector<RTTriangle>& triangles, std::vector<RayVertex>& vertices);

	static void printBVH(std::vector<BVHNode>& nodes);
};

struct RTMaterial {
	glm::vec4 color;
	glm::vec4 properties; // x: type
};

// For metals & lambertians, the y value is the roughness