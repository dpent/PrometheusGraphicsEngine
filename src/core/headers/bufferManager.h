#pragma once

#include "Prometheus.h"
#include "vertex.h"

class Engine;

struct Buffer {
public:
	VkDeviceSize size;
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint32_t offset;
	void* mapped;

	void destroy();
};

class BufferManager {
public:
	static void createFrameBuffers(
		std::vector<VkFramebuffer>& frameBuffers, 
		std::vector<VkImageView>& views, 
		VkExtent2D& extent,
		VkRenderPass& renderPass,
		VkImageView& colorView,
		VkImageView& depthView
	);

	static void createBuffer(
		Buffer& bufferStruct,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties
	);

	static void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, VkDeviceSize size);
	static VkCommandBuffer beginSingleTimeCommands(VkCommandPool& commandPool);
	static void endSingleTimeCommands(VkCommandPool& commandPool, VkCommandBuffer& commandBuffer);

	static void createStagingBuffer(VkDeviceSize size);

	static void createVertexIndexBuffer(VkDeviceSize size);

	static void recordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t& imageIndex);

	template <typename T> static void createSSBO(Buffer& buffer, std::vector<T>& bufferData)
	{

		VkDeviceSize bufferSize = sizeof(T) * bufferData.size();
		buffer.size = bufferSize;

		BufferManager::createBuffer(buffer, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		void* data;
		vkMapMemory(Engine::deviceInfo.logicalDevice, buffer.memory, 0, buffer.size, 0, &data);
		memcpy(data, bufferData.data(), buffer.size);
		//vkUnmapMemory(Engine::deviceInfo.logicalDevice , buffer.memory);
		buffer.mapped = data;
	}

	template <typename T> static void updateSSBO(Buffer& buffer, std::vector<T>& bufferData) {
		memcpy(buffer.mapped, bufferData.data(), buffer.size);
	}
};

struct CommandPool {
public:	

	VkCommandPool pool;
	std::vector<VkCommandBuffer> buffers;

	void initialize();
};

struct VertexData {
public:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};