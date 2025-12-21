#pragma once
#include "gameObject.h"


namespace Prometheus {

	class MapFloor : public GameObject{ //2D object that limits objects from falling through the world
	public:

		MapFloor(std::string texturePath, std::string modelPath, glm::vec3 pos, glm::vec3 scale);

		void instantiate(VkCommandPool& commandPool, VkDevice& device, VkPhysicalDevice& physicalDevice,
			VkQueue& graphicsQueue, int req_comp) override;

		void start() override;
		void update() override;
	};
}