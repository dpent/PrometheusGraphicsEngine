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

	#ifdef RAY_TRACING
	static void createRayTracingSetLayout();
	static void createRayTracingDescriptorPool();
	static void createRayTracingDescriptorSets();
	#endif
};

struct Descriptor {
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> sets;
	VkDescriptorPool pool = VK_NULL_HANDLE;
};

struct SetlessDescriptor {
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	VkDescriptorPool pool = VK_NULL_HANDLE;
};