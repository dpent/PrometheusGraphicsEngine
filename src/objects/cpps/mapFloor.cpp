#include "../headers/mapFloor.h"

using namespace Prometheus;

namespace Prometheus {

	MapFloor::MapFloor(std::string texturePath, std::string modelPath, glm::vec3 pos, glm::vec3 scale) :
		GameObject(texturePath, modelPath){

		this->transform.position = pos;
		this->transform.scale = scale;
	}

	void MapFloor::instantiate(VkCommandPool& commandPool, VkDevice& device, VkPhysicalDevice& physicalDevice,
		VkQueue& graphicsQueue, int req_comp) {

		GameObject::instantiate(commandPool,device,physicalDevice,
			graphicsQueue, req_comp);
	}

	void MapFloor::start() {

		scale(this->transform.scale);
	}

	void MapFloor::update() {
		return;
	}
}