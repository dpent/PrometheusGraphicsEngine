#pragma once

#include "Prometheus.h"

class PipelineManager {
public:

	static void createGraphicsPipeline();
	static void createShadowPipeline();
};

struct Pipeline {
	VkPipelineLayout layout;
	VkPipeline pipeline;
};