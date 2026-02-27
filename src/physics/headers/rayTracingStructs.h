#include "../../core/headers/Prometheus.h"

struct Sphere {
	glm::vec4 center;
	glm::vec4 color;
};

static_assert(sizeof(Sphere) == 32, "Sphere size mismatch");

struct LightSource {
	glm::vec4 position;
	glm::vec4 color;
};