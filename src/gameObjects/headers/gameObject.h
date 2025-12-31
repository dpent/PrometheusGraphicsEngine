#pragma once

#include "../../core/headers/Prometheus.h"
#include "mesh.h"

class GameObject {
public:

	Mesh* mesh;

	GameObject();
	GameObject(Mesh* mesh);
	GameObject(std::string modelFilename);
};