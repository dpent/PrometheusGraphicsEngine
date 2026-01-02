#pragma once

#include "../../core/headers/Prometheus.h"
#include "mesh.h"
#include "transform.h"
#include "material.h"

struct InstanceInfo {
public:
	glm::mat4 modelMatrix;
	uint32_t materialIndex;
	uint32_t padding[3];
};

class GameObject {
public:

	static uint64_t incrementID;

	uint64_t id;
	Mesh* mesh;
	Transform* transform;
	Material* material;
	InstanceInfo* instanceInfo;

	GameObject();
	GameObject(Mesh* mesh, Material* material);
	GameObject(Mesh* mesh, std::string textureFilename);
	GameObject(std::string modelFilename, Material* material);
	GameObject(std::string modelFilename, std::string textureFilename);

	virtual void update();
	virtual void rotate(glm::quat rotation);
};