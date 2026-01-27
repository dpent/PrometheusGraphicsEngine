#pragma once

#include "Prometheus.h"
#include "vertex.h"
#include "pipelineManager.h"
#include "descriptorManager.h"
#include "bufferManager.h"

class Debug {
public:

	static Pipeline debugPipeline;
	static Descriptor debugDescriptor;

	static Buffer debugVertexBuffer;
	static Buffer lineSSBO;

	static std::vector<DebugVertex> debugVertices;
	static std::vector<Line> lines;


	static void init();
	static void drawLine(glm::vec3 start, glm::vec3 finish, glm::vec3 color);
	static void clearDebugData();
	static void drawLineCircle(glm::vec3 center, float radius, uint16_t segments, glm::vec3 color, glm::vec3 notmal);
};