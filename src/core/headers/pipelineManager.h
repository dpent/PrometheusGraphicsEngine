#pragma once

#include "Prometheus.h"

class ParticleEffect;

class PipelineManager {
public:

	static void createGraphicsPipeline();
	static void createShadowPipeline();

	static void createDebugPipeline();

	static void createParticleComputePipeline(ParticleEffect& pEffect);
	static void createParticleGraphicsPipeline();
};

struct Pipeline {
	VkPipelineLayout layout;
	VkPipeline pipeline;
};