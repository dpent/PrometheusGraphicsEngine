#pragma once

#include "../../core/headers/Prometheus.h"
#include "../../core/headers/imageManager.h"
#include "../../core/headers/bufferManager.h"

struct Texture {
public:
	Image image;

	uint32_t mipLevels;
	uint32_t index;

	Texture();
	Texture(std::string filename, CommandPool& command);
};

class Material {
public:

	Texture* texture;

	float metallic;
	float roughness;

	Material();
	Material(Texture* texture, float metallic, float roughness, CommandPool& command);
	Material(std::string filename, float metallic, float roughness, CommandPool& command);
};