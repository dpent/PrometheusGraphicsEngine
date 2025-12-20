#pragma once
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include <unordered_set>
#include <vector>
#include <array>
#include <mutex>

namespace Prometheus{

    class GameObject;

    class Cell{
        public:
        glm::vec3 minCoords;
        glm::vec3 maxCoords;

        std::array<glm::vec3, 8> edges;
        glm::vec3 center;
        glm::vec3 radius;

        std::unordered_set<GameObject*> objects;

        std::mutex objectMutex;

        Cell* children[2][2][2] = { nullptr };
        /*
        0,0,0: Top left forward
        0,0,1: Top right forward
        0,1,0: Top left back
        0,1,1: Top right back
        1,0,0: Bottom left forward
        1,0,1: Bottom right forward
        1,1,0: Bottom left back
        1,1,1: Bottom right back
        */

        static uint8_t maxObjects;

        Cell(glm::vec3 minCoords, glm::vec3 maxCoords);

        Cell();

        Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::unordered_set<GameObject*> objects);

        Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::vector<GameObject*> objects);

        void insert(GameObject* obj);

        void drawSelf();
        void drawAll();
        void split();
        glm::vec3 getCenter();
        std::vector<glm::vec3> getBasicAxes();
    };
}
