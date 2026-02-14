#pragma once

#include "Prometheus.h"

class ParticleEffect;

class PipelineManager {
public:

	static void createGraphicsPipeline();
	static void createShadowPipeline();

	static void createDebugPipeline();
};

struct Pipeline {
	VkPipelineLayout layout;
	VkPipeline pipeline;
};