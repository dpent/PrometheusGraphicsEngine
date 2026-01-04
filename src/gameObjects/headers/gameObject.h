#pragma once

#include "../../core/headers/Prometheus.h"
#include "mesh.h"
#include "transform.h"
#include "material.h"

class GameObject;

struct InstanceInfo {
public:
	glm::mat4 modelMatrix;
	uint32_t materialIndex;
	GameObject* owner;

	InstanceInfo(GameObject* owner);
	std::string toString();
};

static_assert(sizeof(InstanceInfo) % 16 == 0, "Wrong size. This must follow std140 (aligned to 16 bytes)");

struct InitInfo {
	std::string modelFilename = ".";
	Mesh* modelPointer = nullptr;
	std::string textureFilename = ".";
	Material* materialPointer = nullptr;

	InitInfo();
	InitInfo(std::string modelFilename, Mesh* modelPointer, std::string textureFilename, Material* materialPointer);
};

class GameObject {
public:

	static uint64_t incrementID;

	uint64_t id;
	Mesh* mesh;
	Transform* transform;
	Material* material;
	uint32_t instanceIndex;

	GameObject();
	GameObject(Mesh* mesh, Material* material);
	GameObject(Mesh* mesh, std::string textureFilename);
	GameObject(std::string modelFilename, Material* material);
	GameObject(std::string modelFilename, std::string textureFilename);

	virtual void update();
	virtual void rotate(glm::quat rotation);
	virtual void scale(glm::vec3 scale);
	virtual void initialise(InitInfo& info, CommandPool& commandPool);

	static void createInitiasationJob(GameObject* obj, InitInfo* info);
};