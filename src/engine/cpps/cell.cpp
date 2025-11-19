#include "../headers/cell.h"
#include "../../debug/headers/debug.h"


using namespace Prometheus;


namespace Prometheus{

    uint8_t Cell::maxObjects = 10;

    Cell::Cell(glm::vec3 minCoords, glm::vec3 maxCoords){
        this->minCoords = minCoords;
        this->maxCoords = maxCoords;
    }

    Cell::Cell(){}

    Cell::Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::unordered_map<GameObject*, bool> objects){
        this->minCoords = minCoords;
        this->maxCoords = maxCoords;

        this->objects = objects;
    }

    Cell::Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::vector<GameObject*> objects){
        this->minCoords = minCoords;
        this->maxCoords = maxCoords;

        for(size_t i=0; i<objects.size(); i++){
            this->objects[objects[i]] = true;
        }
    }

    void Cell::drawSelf(){
        Debug::drawLine(glm::vec3(maxCoords), glm::vec3(maxCoords.x, maxCoords.y, minCoords.z), COLOR_DARK_GREEN); //TOP PART
        Debug::drawLine(glm::vec3(maxCoords), glm::vec3(minCoords.x, maxCoords.y, maxCoords.z), COLOR_DARK_GREEN);
        Debug::drawLine(glm::vec3(minCoords.x,maxCoords.y,minCoords.z), glm::vec3(maxCoords.x, maxCoords.y, minCoords.z), COLOR_DARK_GREEN);
        Debug::drawLine(glm::vec3(minCoords.x,maxCoords.y,minCoords.z), glm::vec3(minCoords.x, maxCoords.y, maxCoords.z), COLOR_DARK_GREEN);

        Debug::drawLine(glm::vec3(maxCoords), glm::vec3(maxCoords.x, minCoords.y, maxCoords.z), COLOR_DARK_GREEN); //PERPENDICULAR PARTS
        Debug::drawLine(glm::vec3(maxCoords.x, maxCoords.y, minCoords.z), glm::vec3(maxCoords.x, minCoords.y, minCoords.z), COLOR_DARK_GREEN);
        Debug::drawLine(glm::vec3(minCoords.x, maxCoords.y, maxCoords.z), glm::vec3(minCoords.x, minCoords.y, maxCoords.z), COLOR_DARK_GREEN);
        Debug::drawLine(glm::vec3(minCoords.x,maxCoords.y,minCoords.z), glm::vec3(minCoords), COLOR_DARK_GREEN);

        Debug::drawLine(glm::vec3(minCoords), glm::vec3(minCoords.x, minCoords.y, maxCoords.z), COLOR_DARK_GREEN); //BOTTOM PART
        Debug::drawLine(glm::vec3(minCoords), glm::vec3(maxCoords.x, minCoords.y, minCoords.z), COLOR_DARK_GREEN);
        Debug::drawLine(glm::vec3(maxCoords.x,minCoords.y,maxCoords.z), glm::vec3(minCoords.x, minCoords.y, maxCoords.z), COLOR_DARK_GREEN);
        Debug::drawLine(glm::vec3(maxCoords.x,minCoords.y,maxCoords.z), glm::vec3(maxCoords.x, minCoords.y, minCoords.z), COLOR_DARK_GREEN);
    }

    void Cell::insert(GameObject* obj){
        
    }
}