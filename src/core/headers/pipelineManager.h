#pragma once

#include "Prometheus.h"

class ParticleEffect;

class PipelineManager {
public:

	static void createGraphicsPipeline();
	static void createShadowPipeline();

	static void createDebugPipeline();
	#ifdef RAY_TRACING
	static void createRayTracingPipeline();
	#endif
};

struct Pipeline {
	VkPipelineLayout layout;
	VkPipeline pipeline;
};