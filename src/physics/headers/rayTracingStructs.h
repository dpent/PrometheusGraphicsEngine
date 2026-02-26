#include "../../core/headers/Prometheus.h"

struct Sphere {
	glm::vec4 center;
	glm::vec4 color;
};

static_assert(sizeof(Sphere) == 32, "Sphere size mismatch");