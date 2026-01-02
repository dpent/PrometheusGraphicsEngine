#pragma once

#include "../../core/headers/Prometheus.h"
#include "../../core/headers/imageManager.h"

struct Texture {
public:
	Image image;

	uint32_t mipLevels;
	uint32_t index;

	Texture();
	Texture(std::string filename);
};

class Material {
public:

	Texture* texture;

	float metallic;
	float roughness;

	Material();
	Material(Texture* texture, float metallic, float roughness);
	Material(std::string filename, float metallic, float roughness);
};