#include "../headers/rayTracingStructs.h"
#include "../../core/headers/engine.h"

void BVH::createBVH(std::vector<RTTriangle>& triangles, std::vector<BVHNode>& nodes, glm::vec3& maxCoords, glm::vec3& minCoords, std::vector<glm::vec3> centroids, std::vector<RayVertex>& vertices)
{
	if (triangles.empty()) {
		return;
	}

	BVHNode rootNode{
		.childIndices = glm::ivec4(-1, -1, 0, static_cast<uint32_t>(triangles.size())),
		.minBounds = glm::vec4(minCoords, 0.0f),
		.maxBounds = glm::vec4(maxCoords, 0.0f)
	};

	nodes.push_back(rootNode);

	if (static_cast<uint32_t>(triangles.size()) > BVH::MAX_TRIANGLES_PER_NODE && BVH::MAX_DEPTH > 0) {
		BVH::splitNode(nodes[0], triangles, nodes, centroids, 1, vertices);
	}

	return;

}

void BVH::splitNode(BVHNode& node, std::vector<RTTriangle>& triangles, std::vector<BVHNode>& nodes, std::vector<glm::vec3>& centroids, uint32_t depth, std::vector<RayVertex>& vertices) {

	node.childIndices.x = static_cast<uint32_t>(nodes.size());
	node.childIndices.y = static_cast<uint32_t>(nodes.size() + 1);

	float deltaX = node.maxBounds.x - node.minBounds.x;
	float deltaY = node.maxBounds.y - node.minBounds.y;
	float deltaZ = node.maxBounds.z - node.minBounds.z;

	float deltas[3] = { deltaX, deltaY, deltaZ };

	int axis = 0;
	if (deltas[1] > deltas[axis]) axis = 1;
	if (deltas[2] > deltas[axis]) axis = 2;

	float split = (node.minBounds[axis] + node.maxBounds[axis]) * 0.5f;

	glm::vec4 leftMax = node.maxBounds;
	leftMax[axis] = split;

	glm::vec4 rightMin = node.minBounds;
	rightMin[axis] = split;


	BVHNode leftChild{
		.childIndices = glm::ivec4(-1, -1, node.childIndices.z, 0),
		.minBounds = node.minBounds,
		.maxBounds = leftMax,
	};

	size_t i = node.childIndices.z;
	size_t j = i + node.childIndices.w - 1;
	//std::cout << "Depth is " << depth << " and there are "<<centroids.size()<<" centroids with i being "<< i << " and j being "<<j << std::endl;

	while (i <= j) {
		//std::cout << i << " " << j << std::endl;
		if (centroids[i][axis] < leftMax[axis]) {
			i++;
		}
		else {
			std::swap(centroids[i], centroids[j]);
			std::swap(triangles[i], triangles[j]);
			j--;
		}
	}

	uint32_t leftCount = static_cast<uint32_t>(i - node.childIndices.z);
	leftChild.childIndices.w = leftCount;

	BVH::extendBoundaries(leftChild, triangles, vertices);

	BVHNode rightChild{
		.childIndices = glm::ivec4(-1, -1, node.childIndices.z + leftChild.childIndices.w, node.childIndices.w - leftCount),
		.minBounds = rightMin,
		.maxBounds = node.maxBounds,
	};
	BVH::extendBoundaries(rightChild, triangles, vertices);

	node.childIndices.w = 0; //Mark as internal
	node.childIndices.z = -1;

	nodes.push_back(leftChild);
	nodes.push_back(rightChild);

	if (nodes[node.childIndices.x].childIndices.w > BVH::MAX_TRIANGLES_PER_NODE && depth < BVH::MAX_DEPTH){
		
		splitNode(nodes[node.childIndices.x], triangles, nodes, centroids, depth + 1, vertices);
	}

	if (nodes[node.childIndices.y].childIndices.w > BVH::MAX_TRIANGLES_PER_NODE && depth < BVH::MAX_DEPTH){

		splitNode(nodes[node.childIndices.y], triangles, nodes, centroids, depth + 1, vertices);
	}
}

std::vector<glm::vec3> BVH::caclulateCentroids(std::vector<RTTriangle>& triangles, std::vector<RayVertex>& vertices) {

	std::vector<glm::vec3> centroids;
	
	for (const auto& triangle : triangles) {
		glm::vec3 pos0 = vertices[triangle.vertexIndices.x].position;
		glm::vec3 pos1 = vertices[triangle.vertexIndices.y].position;
		glm::vec3 pos2 = vertices[triangle.vertexIndices.z].position;

		glm::vec3 centroid = (pos0 + pos1 + pos2) / 3.0f;
		centroids.push_back(centroid);
	}

	return centroids;
}

void BVH::extendBoundaries(BVHNode& node, std::vector<RTTriangle>& triangles, std::vector<RayVertex>& vertices) {
	uint32_t start = node.childIndices.z;
	uint32_t numTriangles = node.childIndices.w;

	glm::vec4 max;
	glm::vec4 min;

	for(uint32_t i = start; i < start + numTriangles; i++){
		
		RayVertex v0 = vertices[triangles[i].vertexIndices[0]];
		RayVertex v1 = vertices[triangles[i].vertexIndices[1]];
		RayVertex v2 = vertices[triangles[i].vertexIndices[2]];

		max = glm::max(max, glm::max(v0.position, v1.position, v2.position));
		min = glm::min(min, glm::min(v0.position, v1.position, v2.position));
	}

	node.maxBounds = glm::max(node.maxBounds, max);
	node.minBounds = glm::min(node.minBounds, min);
}

void BVH::printBVH(std::vector<BVHNode>& nodes) {

	std::cout << "\n===== BVH NODES (" << nodes.size() << ") =====\n\n";

	for (size_t i = 0; i < nodes.size(); i++)
	{
		const BVHNode& node = nodes[i];

		std::cout << "---------------------------------\n";
		std::cout << "Node " << i << "\n";

		bool isLeaf = node.childIndices.z != -1;

		if (isLeaf)
		{
			std::cout << "Type: LEAF\n";
			std::cout << "First Triangle: " << node.childIndices.z << "\n";
			std::cout << "Triangle Count: " << node.childIndices.w << "\n";
		}
		else
		{
			std::cout << "Type: INTERNAL\n";
			std::cout << "Left Child : " << node.childIndices.x << "\n";
			std::cout << "Right Child: " << node.childIndices.y << "\n";
		}

		std::cout << "\nBounds:\n";

		std::cout << "  Min: ("
			<< node.minBounds.x << ", "
			<< node.minBounds.y << ", "
			<< node.minBounds.z << ")\n";

		std::cout << "  Max: ("
			<< node.maxBounds.x << ", "
			<< node.maxBounds.y << ", "
			<< node.maxBounds.z << ")\n";

		std::cout << "\n";
	}

	std::cout << "===============================\n";

}