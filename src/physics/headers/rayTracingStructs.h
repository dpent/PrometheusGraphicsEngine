#include "../../core/headers/Prometheus.h"

struct RayVertex {
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 normal;

	bool operator==(const RayVertex& other) const
	{
		return position == other.position &&
			color == other.color;
	}
};

namespace std {
	template<> struct hash<RayVertex> {
		size_t operator()(RayVertex const& vertex) const {
			return ((hash<glm::vec4>()(vertex.position) ^
				(hash<glm::vec4>()(vertex.color) << 1)) >> 1);
		}
	};
}

struct RTMaterial {
	glm::vec4 color;
	glm::vec4 properties; // x: type
};

struct RTTriangle {
	glm::ivec4 vertexIndices; // w: material index
};