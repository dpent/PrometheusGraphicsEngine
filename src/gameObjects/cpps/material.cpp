#include "../headers/material.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../core/headers/stb_image.h"
#include "../../core/headers/engine.h"

Texture::Texture(){}

Texture::Texture(std::string filename) {

    Engine::textures.push_back(this);

    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load((std::filesystem::path(TEXTURE_DIR) / filename).lexically_normal().string().c_str(), &texWidth, &texHeight, &texChannels, 4);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (!pixels) {

        const char* error = stbi_failure_reason();
        std::cout << "STBI Error: " << (error ? error : "Unknown error") << std::endl;
        std::cout << "Failed to load: " << filename << std::endl;

        throw std::runtime_error("failed to load texture image!");
    }

	if (imageSize > Engine::stagingBuffer.size) {
        BufferManager::createStagingBuffer(imageSize);
    }

    void* data;
    vkMapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(Engine::deviceInfo.logicalDevice, Engine::stagingBuffer.memory);

    stbi_image_free(pixels);

    ImageManager::createImage(texWidth,
        texHeight,
        VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        this->image.image,
        this->image.memory,
        mipLevels,
        VK_SAMPLE_COUNT_1_BIT
    );

    this->mipLevels = mipLevels;

    ImageManager::transitionImageLayout(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, Engine::command.pool);

    ImageManager::copyBufferToImage(Engine::stagingBuffer.buffer, image.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
        Engine::command.pool);

    ImageManager::generateMipMaps(image.image, texWidth, texHeight, mipLevels,VK_FORMAT_R8G8B8A8_SRGB, Engine::command.pool);

    ImageManager::createImageView(image.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, VK_IMAGE_VIEW_TYPE_2D, 1, 0, image.view);
}

Material::Material() {}

Material::Material(Texture* texture, float metallic, float roughness) {
    this->texture = texture;
    this->metallic = metallic;
    this->roughness = roughness;

    Engine::materials.push_back(this);
}

Material::Material(std::string filename, float metallic, float roughness) {
    this->texture = new Texture(filename);
    this->metallic = metallic;
    this->roughness = roughness;

    Engine::materials.push_back(this);
}