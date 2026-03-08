#include "../headers/rayTracingStructs.h"
#include "../../core/headers/engine.h"

void BVH::buildBVH(
	std::vector<RTTriangle>& triangles,
	uint32_t triangleOffset,
	std::vector<BVHNode>& nodes,
	glm::vec3 maxCoords,
	glm::vec3 minCoords,
	std::vector<glm::vec3> centroids,
	std::vector<RayVertex>& vertices
) {
	BVHNode root{
		.childIndices = glm::ivec4(-1, -1, triangleOffset, static_cast<uint32_t>(triangles.size()) - triangleOffset),
		.minBounds = glm::vec4(minCoords, 0.0f),
		.maxBounds = glm::vec4(maxCoords, 0.0f)
	};

	nodes.push_back(root);

	if (root.childIndices.w > BVH::MAX_TRIANGLES_PER_NODE && BVH::MAX_DEPTH > 0) {

		BVH::splitBVHNode(static_cast<uint32_t>(nodes.size() - 1), triangles, triangleOffset, nodes, centroids, vertices, 1);
	}

}

void BVH::splitBVHNode(
	uint32_t nodeIndex,
	std::vector<RTTriangle>& triangles,
	uint32_t triangleOffset,
	std::vector<BVHNode>& nodes,
	std::vector<glm::vec3>& centroids,
	std::vector<RayVertex>& vertices,
	uint32_t depth
) {

	float deltaX = nodes[nodeIndex].maxBounds.x - nodes[nodeIndex].minBounds.x;
	float deltaY = nodes[nodeIndex].maxBounds.y - nodes[nodeIndex].minBounds.y;
	float deltaZ = nodes[nodeIndex].maxBounds.z - nodes[nodeIndex].minBounds.z;

	float deltas[3] = { deltaX, deltaY, deltaZ };

	int axis = 0;
	if (deltas[1] > deltas[axis]) axis = 1;
	if (deltas[2] > deltas[axis]) axis = 2;

	float split = getCentroidMidPoint(nodes[nodeIndex], axis, centroids, glm::vec3(nodes[nodeIndex].maxBounds));

	glm::vec4 leftMax = nodes[nodeIndex].maxBounds;
	leftMax[axis] = split;

	glm::vec4 rightMin = nodes[nodeIndex].minBounds;
	rightMin[axis] = split;

	BVHNode leftChild{
		.childIndices = glm::ivec4(-1, -1, triangleOffset, 0),
		.minBounds = nodes[nodeIndex].minBounds,
		.maxBounds = leftMax,
	};

	size_t i = triangleOffset;
	size_t j = i + nodes[nodeIndex].childIndices.w - 1;

	while (i <= j) {
		if (centroids[i][axis] < leftMax[axis]) {
			i++;
		}
		else {
			std::swap(centroids[i], centroids[j]);
			std::swap(triangles[i], triangles[j]);
			j--;
		}
	}

	uint32_t leftCount = static_cast<uint32_t>(i - triangleOffset);
	leftChild.childIndices.w = leftCount;

	if (leftCount == 0 || leftCount == nodes[nodeIndex].childIndices.w)
		return;

	BVH::extendBoundaries(leftChild, triangles, vertices);

	BVHNode rightChild{
		.childIndices = glm::ivec4(-1, -1, triangleOffset + leftChild.childIndices.w, nodes[nodeIndex].childIndices.w - leftCount),
		.minBounds = rightMin,
		.maxBounds = nodes[nodeIndex].maxBounds,
	};

	BVH::extendBoundaries(rightChild, triangles, vertices);

	nodes[nodeIndex].childIndices.w = 0; //Mark as internal
	nodes[nodeIndex].childIndices.z = -1;

	nodes[nodeIndex].childIndices.x = static_cast<uint32_t>(nodes.size());
	nodes[nodeIndex].childIndices.y = static_cast<uint32_t>(nodes.size() + 1);
	nodes.push_back(leftChild);
	nodes.push_back(rightChild);


	if (nodes[nodes[nodeIndex].childIndices.x].childIndices.w > BVH::MAX_TRIANGLES_PER_NODE && depth < BVH::MAX_DEPTH) {
		
		splitBVHNode(nodes[nodeIndex].childIndices.x, triangles, leftChild.childIndices.z, nodes, centroids, vertices, depth + 1);
	}

	if (nodes[nodes[nodeIndex].childIndices.y].childIndices.w > BVH::MAX_TRIANGLES_PER_NODE && depth < BVH::MAX_DEPTH) {

		splitBVHNode(nodes[nodeIndex].childIndices.y, triangles, rightChild.childIndices.z, nodes, centroids, vertices, depth + 1);
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

	glm::vec4 max = glm::vec4(-FLT_MAX);
	glm::vec4 min = glm::vec4(FLT_MAX);

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

float BVH::getCentroidMidPoint(BVHNode& node, int axis, std::vector<glm::vec3>& centroids, glm::vec3 maxBounds) {

	size_t start = node.childIndices.z;
	size_t count = node.childIndices.w;

	size_t mid = start + count / 2;

	std::nth_element(
		centroids.begin() + start,
		centroids.begin() + mid,
		centroids.begin() + start + count,
		[axis](const glm::vec3& a, const glm::vec3& b) {
			return a[axis] < b[axis];
		});
		
	float split = centroids[mid][axis];
	int leftCount = 0;
	int rightCount = 0;

	for (size_t i = start; i < start + count; i++) {
	
		if (centroids[i][axis] < split) {
			leftCount++;
		}
		else if (centroids[i][axis] > split) {
			rightCount++;
		}

		if (leftCount != 0 && rightCount != 0){
			break;
		}
	}

	if (leftCount == 0 || rightCount == 0){

		float minC = centroids[start][axis];
		float maxC = centroids[start][axis];
		for (size_t i = start; i < start + count; i++) {
			minC = std::min(minC, centroids[i][axis]);
			maxC = std::max(maxC, centroids[i][axis]);
		}
		split = 0.5f * (minC + maxC);

	}
	return split;
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

void TLAS::addToTLAS(std::vector<TLASNode>& tlasNodes, uint32_t firstNode, uint32_t nodeCount, glm::vec3& maxCoords, glm::vec3& minCoords) {

	TLASNode node{
		.nodeIndices = glm::ivec4(firstNode, nodeCount, 0, 0),
		.minBounds = glm::vec4(minCoords, 0.0f),
		.maxBounds = glm::vec4(maxCoords, 0.0f)
	};

	tlasNodes.push_back(node);

}