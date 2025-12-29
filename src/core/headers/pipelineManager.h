#pragma once

#include "Prometheus.h"

class PipelineManager {
public:

	static void createGraphicsPipeline();
};

struct Pipeline {
	VkPipelineLayout layout;
	VkPipeline pipeline;
};