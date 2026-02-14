#pragma once

#include "Prometheus.h"

class ParticleEffect;

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

	static void createParticleSetLayout();
	static void createParticleDescriptorPool();
};

struct Descriptor {
	VkDescriptorSetLayout layout;
	std::vector<VkDescriptorSet> sets;
	VkDescriptorPool pool;
};

struct SetlessDescriptor {
	VkDescriptorSetLayout layout;
	VkDescriptorPool pool;
};