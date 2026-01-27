#pragma once

#include "Prometheus.h"

class DescriptorManager {
public:
	static void createGraphicsDescriptorSetLayout();
	static void createGraphicsDescriptorPool();
	static void createGraphicsDescriptorSets();

	static void createShadowLightsSetLayout();
	static void createShadowLightsPool();
	static void createShadowLightsSets();

	static void createDebugSetLayout();
	static void createDebugDescriptorPool();
	static void createDebugDescriptorSets();
};

struct Descriptor {
	VkDescriptorSetLayout layout;
	std::vector<VkDescriptorSet> sets;
	VkDescriptorPool pool;
};