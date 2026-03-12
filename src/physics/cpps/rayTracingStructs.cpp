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

	float split = getCentroidMidPoint(nodes[nodeIndex], axis, centroids);

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

float BVH::getCentroidMidPoint(BVHNode& node, int axis, std::vector<glm::vec3>& centroids) {

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

void TLAS::buildTLAS() {

	glm::vec4 max = glm::vec4(-FLT_MAX);
	glm::vec4 min = glm::vec4(FLT_MAX);

	std::vector<glm::vec4> centroids;

	for (size_t i = 0; i < Engine::tlasNodes.size(); i++) {
		max = glm::max(max, Engine::tlasNodes[i].maxBounds);
		min = glm::min(min, Engine::tlasNodes[i].minBounds);

		centroids.push_back((Engine::tlasNodes[i].maxBounds + Engine::tlasNodes[i].minBounds) * 0.5f);
	}

	std::vector<TLASNode> tlasTree;
	TLASNode root{
		.nodeIndices = glm::ivec4(-1,-1,-1,-1),
		.minBounds = min,
		.maxBounds = max
	};

	tlasTree.push_back(root);

	if (Engine::tlasNodes.size() > 2) {
		TLAS::splitTLASNode(0, tlasTree, 0, static_cast<uint32_t>(Engine::tlasNodes.size() - 1), centroids);
	}
	else if(Engine::tlasNodes.size() == 2) {
		tlasTree[0].nodeIndices.z = 1;
		tlasTree[0].nodeIndices.w = 2;

		tlasTree.push_back(Engine::tlasNodes[0]);
		tlasTree.push_back(Engine::tlasNodes[1]);
	}
	else if (Engine::tlasNodes.size() == 1) {
		tlasTree = Engine::tlasNodes;
	}

	Engine::tlasNodes = tlasTree;
}

void TLAS::splitTLASNode(uint32_t nodeIndex, std::vector<TLASNode>& tlasTree, uint32_t startIndex, uint32_t endIndex, std::vector<glm::vec4>& centroids) {

	if (endIndex - startIndex + 1 == 2) {

		tlasTree[nodeIndex].nodeIndices.z = static_cast<uint32_t>(tlasTree.size());
		tlasTree.push_back(Engine::tlasNodes[startIndex]);
		tlasTree[nodeIndex].nodeIndices.w = static_cast<uint32_t>(tlasTree.size());
		tlasTree.push_back(Engine::tlasNodes[endIndex]);

		return;
	}

	float deltaX = tlasTree[nodeIndex].maxBounds.x - tlasTree[nodeIndex].minBounds.x;
	float deltaY = tlasTree[nodeIndex].maxBounds.y - tlasTree[nodeIndex].minBounds.y;
	float deltaZ = tlasTree[nodeIndex].maxBounds.z - tlasTree[nodeIndex].minBounds.z;

	float deltas[3] = { deltaX, deltaY, deltaZ };

	int axis = 0;
	if (deltas[1] > deltas[axis]) axis = 1;
	if (deltas[2] > deltas[axis]) axis = 2;

	size_t i = startIndex;
	size_t j = endIndex;

	if (deltaX == deltaY && deltaX == deltaZ && !TLAS::notAllCentroidsOnOnePoint(centroids, axis, startIndex, endIndex)) {

		i = static_cast<size_t>((startIndex + endIndex) * 0.5f);
		j = i - 1;
	}

	float split = findBestSplit(startIndex, endIndex, axis, centroids);

	glm::vec4 leftMax = tlasTree[nodeIndex].maxBounds;
	leftMax[axis] = split;

	glm::vec4 rightMin = tlasTree[nodeIndex].minBounds;
	rightMin[axis] = split;

	TLASNode leftChild{
		.nodeIndices = glm::ivec4(-1, -1, -1, -1),
		.minBounds = tlasTree[nodeIndex].minBounds,
		.maxBounds = leftMax,
	};

	while (i <= j) {

		if (centroids[i][axis] < leftMax[axis]) {
			i++;
		}
		else {
			std::swap(centroids[i], centroids[j]);
			std::swap(Engine::tlasNodes[i], Engine::tlasNodes[j]);
			if (j == startIndex) {
				break;
			}
			j--;
		}
	}

	TLAS::extendBoundaries(leftChild, startIndex, static_cast<uint32_t>(j));
	TLASNode rightChild{
		.nodeIndices = glm::ivec4(-1, -1, -1, -1),
		.minBounds = rightMin,
		.maxBounds = tlasTree[nodeIndex].maxBounds,
	};
	TLAS::extendBoundaries(rightChild, static_cast<uint32_t>(j + 1), endIndex);

	tlasTree[nodeIndex].nodeIndices.z = static_cast<uint32_t>(tlasTree.size());
	if (startIndex == j) {

		tlasTree.push_back(Engine::tlasNodes[startIndex]);
	}
	else {
		tlasTree.push_back(leftChild);

		TLAS::splitTLASNode(tlasTree[nodeIndex].nodeIndices.z, tlasTree, startIndex, static_cast<uint32_t>(j), centroids);
	}

	tlasTree[nodeIndex].nodeIndices.w = static_cast<uint32_t>(tlasTree.size());
	if (j + 1 == endIndex) {

		tlasTree.push_back(Engine::tlasNodes[endIndex]);
	}
	else {

		tlasTree.push_back(rightChild);

		TLAS::splitTLASNode(tlasTree[nodeIndex].nodeIndices.w, tlasTree, static_cast<uint32_t>(j + 1), endIndex, centroids);
	}
}

float TLAS::findBestSplit(uint32_t startIndex, uint32_t endIndex, int axis, std::vector<glm::vec4>& centroids) {
	
	size_t mid = (startIndex + endIndex) / 2;

	std::nth_element(
		centroids.begin() + startIndex,
		centroids.begin() + mid,
		centroids.begin() + endIndex,
		[axis](const glm::vec3& a, const glm::vec3& b) {
			return a[axis] < b[axis];
		});

	float split = centroids[mid][axis];
	int leftCount = 0;
	int rightCount = 0;

	for (size_t i = startIndex; i < endIndex; i++) { //Debug purposes

		if (centroids[i][axis] < split) {
			leftCount++;
		}
		else if (centroids[i][axis] > split) {
			rightCount++;
		}

		if (leftCount != 0 && rightCount != 0) {
			break;
		}
	}

	return split;
}

void TLAS::extendBoundaries(TLASNode& node, uint32_t startIndex, uint32_t endIndex) {

	glm::vec4 max = glm::vec4(-FLT_MAX);
	glm::vec4 min = glm::vec4(FLT_MAX);

	for (uint32_t i = startIndex; i < endIndex; i++) {

		max = glm::max(max, Engine::tlasNodes[i].maxBounds);
		min = glm::min(min, Engine::tlasNodes[i].minBounds);
	}

	node.maxBounds = glm::max(node.maxBounds, max);
	node.minBounds = glm::min(node.minBounds, min);
}

bool TLAS::notAllCentroidsOnOnePoint(std::vector<glm::vec4>& centroids, int axis, uint32_t startIndex, uint32_t endIndex) {

	float val = centroids[0][axis];
	for (uint32_t i = startIndex; i < endIndex; i++) {
		if (centroids[i][axis] != val) {
			return true;
		}
	}

	return false;
}

void TLAS::printTLAS(std::vector<TLASNode>& nodes) {

	std::cout << "\n===== TLAS NODES (" << nodes.size() << ") =====\n\n";

	for (size_t i = 0; i < nodes.size(); i++)
	{
		const TLASNode& node = nodes[i];

		std::cout << "---------------------------------\n";
		std::cout << "Node " << i << "\n";

		bool isLeaf = node.nodeIndices.x != -1;

		if (isLeaf)
		{
			std::cout << "Type: LEAF\n";
			std::cout << "First node: " << node.nodeIndices.x << "\n";
			std::cout << "Node Count: " << node.nodeIndices.y << "\n";
		}
		else
		{
			std::cout << "Type: INTERNAL\n";
			std::cout << "Left Child : " << node.nodeIndices.z << "\n";
			std::cout << "Right Child: " << node.nodeIndices.w << "\n";
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