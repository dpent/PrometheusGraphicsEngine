#pragma once

#include "../../core/headers/Prometheus.h"
#include "mesh.h"
#include "transform.h"

class GameObject {
public:

	static uint64_t incrementID;

	uint64_t id;
	Mesh* mesh;
	Transform* transform;

	GameObject();
	GameObject(Mesh* mesh);
	GameObject(std::string modelFilename);
};