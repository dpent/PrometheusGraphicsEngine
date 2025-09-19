#include "../headers/textureManager.h"
#include "../headers/engine.h"
#include "../headers/bufferManager.h"


using namespace Prometheus;

namespace Prometheus{
    void TextureManager::createTextureImage(const char * filename, int req_comp, VkDevice& device, VkPhysicalDevice& physicalDevice){
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels, req_comp);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        BufferManager::createBuffer(imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffer, 
            stagingBufferMemory,
            device,
            physicalDevice);

        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(device, stagingBufferMemory);

        stbi_image_free(pixels);
    }
}