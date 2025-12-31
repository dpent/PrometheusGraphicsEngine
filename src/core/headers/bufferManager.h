#pragma once

#include "Prometheus.h"
#include "vertex.h"

struct Buffer {
public:
	VkDeviceSize size;
	VkBuffer buffer;
	VkDeviceMemory memory;
	uint32_t offset;

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