#pragma once

#include "../../core/headers/Prometheus.h"
#include "particle.h"
#include "../../core/headers/bufferManager.h"
#include "../../core/headers/pipelineManager.h"
#include "../../core/headers/descriptorManager.h"
#include "../../core/headers/imageManager.h"

struct ComputePushContant {

	virtual const void* data() { return 0; };
	virtual const uint32_t size() { return 0; };
	virtual void update();

	ComputePushContant();
};

struct SmokePCData {
	float maxHeight;
	float windSpeed;
	glm::vec4 emitterPosition;
	glm::vec4 windDirection;
};

struct SmokeComputePushConstant : ComputePushContant {
	
	SmokePCData pc;

	const void* data() override;

	const uint32_t size() override;
	void update() override;

	SmokeComputePushConstant();
	SmokeComputePushConstant(float maxHeight, float windSpeed, glm::vec4 emitterPosition, glm::vec4 windDirection);
};

class ParticleEffect {
public:

	std::vector<Particle> particles;
	std::vector<Buffer> buffers;
	std::string computeShaderFilename;

	std::string vertexShaderFilename;
	std::string fragmentShaderFilename;

	std::vector<VkDescriptorSet> sets;

	Descriptor graphicsDescriptor;

	Pipeline graphicsPipeline;
	Pipeline pipeline;

	ComputePushContant* computePushConstant;

	ParticleEffect();
	ParticleEffect(uint64_t numParticles, std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC);
	ParticleEffect(std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC);

	virtual void createGraphicsDescriptor();
	virtual void addParticles(uint64_t numParticles);
	virtual void createGraphicsPipeline();
	virtual void createComputeDescriptorSets();
	virtual void createComputePipeline();
	virtual void update();

	ComputePushContant* getComputePushConstants();
	void createBuffers();
};

class SmokeEffect : public ParticleEffect {
public:

	Image image;
	uint32_t mipLevels;

	SmokeEffect();
	SmokeEffect(uint64_t numParticles, std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC);
	SmokeEffect(std::string computeShaderFilename, std::string vertexShaderFilename, std::string fragmentShaderFilename, ComputePushContant* cPC);

	void createGraphicsDescriptor() override;
	void addParticles(uint64_t numParticles) override;
	void createGraphicsPipeline() override;
	void createComputeDescriptorSets() override;
	void createComputePipeline() override;
	void update() override;

	void loadImage();
};