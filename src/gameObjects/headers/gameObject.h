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
	uint32_t hasNormal = 0;
	uint8_t  padding[8];

	InstanceInfo(GameObject* owner);
	std::string toString();
};

static_assert(sizeof(InstanceInfo) % 16 == 0 && sizeof(InstanceInfo) == 96, "Wrong size. This must follow std140 (aligned to 16 bytes)");

struct InitInfo {
	std::string modelFilename = ".";
	Mesh* modelPointer = nullptr;
	std::string textureFilename = ".";
	Material* materialPointer = nullptr;
	std::string normalMapFilename = ".";

	InitInfo();
	InitInfo(std::string modelFilename, Mesh* modelPointer, std::string textureFilename, Material* materialPointer, std::string normalMapFilename);
};

class GameObject {
public:

	static uint64_t incrementID;

	uint64_t id;
	Mesh* mesh;
	Transform* transform;
	Material* material;
	uint32_t instanceIndex;

	uint32_t hasNormalMap = 0;

	GameObject* next = nullptr;
	GameObject* prev = nullptr;

	GameObject();
	GameObject(Mesh* mesh, Material* material);
	GameObject(Mesh* mesh, std::string textureFilename, std::string normalMapFilename);
	GameObject(std::string modelFilename, Material* material);
	GameObject(std::string modelFilename, std::string textureFilename, std::string normalMapFilename);

	virtual void update();
	virtual void rotate(glm::quat rotation);
	virtual void scale(glm::vec3 scale);
	virtual void initialise(InitInfo& info, CommandPool& commandPool, Buffer& stagingBuffer);

	static void createInitiasationJob(GameObject* obj, InitInfo* info);
	static void createDeleteJob(GameObject* obj);
	
	~GameObject();
};