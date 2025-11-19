#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include <unordered_map>
#include "../../objects/headers/gameObject.h"
#include <vector>

namespace Prometheus{

    class Cell{
        public:
        glm::vec3 minCoords;
        glm::vec3 maxCoords;

        std::unordered_map<GameObject*, bool> objects;

        Cell* children[2][2];

        static uint8_t maxObjects;

        Cell(glm::vec3 minCoords, glm::vec3 maxCoords);

        Cell();

        Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::unordered_map<GameObject*, bool> objects);

        Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::vector<GameObject*> objects);

        void insert(GameObject* obj);

        void drawSelf();
    };
}
