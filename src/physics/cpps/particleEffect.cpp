#include "../headers/particleEffect.h"
#include "../../core/headers/engine.h"
#include <random>
#include "../../core/headers/descriptorManager.h"

ParticleEffect::ParticleEffect() {}

ParticleEffect::ParticleEffect(uint64_t numParticles, std::string shaderFilename) {
	this->shaderFilename = shaderFilename;
	particles.reserve(numParticles);

	std::default_random_engine gen;
	std::uniform_real_distribution<float> distribution(0.0f, 10.0f);

	for (uint64_t i = 0; i < numParticles; i++) {
		
		glm::vec3 position{ distribution(gen), distribution(gen), distribution(gen) };
		glm::vec3 color{ distribution(gen) / 10.0f, distribution(gen) / 10.0f, distribution(gen) / 10.0f };
		glm::vec3 velocity{ 0.0f };

		Particle p{position,color,velocity};

		particles.push_back(p);
	}

	buffers.resize(Engine::MAX_FRAMES_IN_FLIGHT);

	VkDeviceSize bufferSize = sizeof(Particle) * particles.size();


	if (bufferSize > Engine::stagingBuffer.size) {
		BufferManager::createStagingBuffer(bufferSize, Engine::stagingBuffer);
	}

	void* data;
	vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, bufferSize, 0, &data);
	memcpy(data, particles.data(), bufferSize);
	vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

	for (size_t i = 0; i < buffers.size(); i++) {

		BufferManager::createParticleSSBO(buffers[i], Engine::stagingBuffer, particles);
		BufferManager::copyBuffer(Engine::stagingBuffer, buffers[i], bufferSize);
	}

	DescriptorManager::createParticleDescriptorSets(*this);

	PipelineManager::createParticleComputePipeline(*this);
}

ParticleEffect::ParticleEffect(std::string shaderFilename) {
	this->shaderFilename = shaderFilename;
}