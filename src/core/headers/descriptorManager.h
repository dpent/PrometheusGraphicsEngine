#pragma once

#include "Prometheus.h"

class DescriptorManager {
public:
	static void createGraphicsDescriptorSetLayout();
	static void createGraphicsDescriptorPool();
	static void createGraphicsDescriptorSets();
};

struct Descriptor {
	VkDescriptorSetLayout layout;
	std::vector<VkDescriptorSet> sets;
	VkDescriptorPool pool;
};