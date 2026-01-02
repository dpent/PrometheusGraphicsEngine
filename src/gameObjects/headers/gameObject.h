#pragma once

#include "../../core/headers/Prometheus.h"
#include "mesh.h"
#include "transform.h"
#include "material.h"

class GameObject {
public:

	static uint64_t incrementID;

	uint64_t id;
	Mesh* mesh;
	Transform* transform;
	Material* material;

	GameObject();
	GameObject(Mesh* mesh, Material* material);
	GameObject(Mesh* mesh, std::string textureFilename);
	GameObject(std::string modelFilename, Material* material);
	GameObject(std::string modelFilename, std::string textureFilename);
};