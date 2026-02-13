#pragma once

#include "../../core/headers/Prometheus.h"
#include "particle.h"
#include "../../core/headers/bufferManager.h"
#include "../../core/headers/pipelineManager.h"

class ParticleEffect {
public:

	std::vector<Particle> particles;
	std::vector<Buffer> buffers;
	std::string shaderFilename;

	std::vector<VkDescriptorSet> sets;
	Pipeline pipeline;

	ParticleEffect();
	ParticleEffect(uint64_t numParticles, std::string shaderFilename);
	ParticleEffect(std::string shaderFilename);
};