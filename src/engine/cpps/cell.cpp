#include "../headers/cell.h"
#include "../../debug/headers/debug.h"
#include "../../objects/headers/gameObject.h"
#include "../headers/engine.h"
#include "../../physics/headers/collision.h"

using namespace Prometheus;


namespace Prometheus{

    uint8_t Cell::maxObjects = 10;

    Cell::Cell(glm::vec3 minCoords, glm::vec3 maxCoords){
        this->minCoords = minCoords;
        this->maxCoords = maxCoords;

        edges[0] = glm::vec3(minCoords.x, maxCoords.y, maxCoords.z);
        edges[1] = glm::vec3(maxCoords.x, maxCoords.y, maxCoords.z);
        edges[2] = glm::vec3(minCoords.x, maxCoords.y, minCoords.z);
        edges[3] = glm::vec3(maxCoords.x, maxCoords.y, minCoords.z);
        edges[4] = glm::vec3(minCoords.x, minCoords.y, maxCoords.z);
        edges[5] = glm::vec3(maxCoords.x, minCoords.y, maxCoords.z);
        edges[6] = glm::vec3(minCoords.x, minCoords.y, minCoords.z);
        edges[7] = glm::vec3(maxCoords.x, minCoords.y, minCoords.z);

        center = getCenter();
        radius = (maxCoords - minCoords) * 0.5f;
    }

    Cell::Cell(){}

    Cell::Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::unordered_set<GameObject*> objects){
        this->minCoords = minCoords;
        this->maxCoords = maxCoords;

        edges[0] = glm::vec3(minCoords.x, maxCoords.y, maxCoords.z);
        edges[1] = glm::vec3(maxCoords.x, maxCoords.y, maxCoords.z);
        edges[2] = glm::vec3(minCoords.x, maxCoords.y, minCoords.z);
        edges[3] = glm::vec3(maxCoords.x, maxCoords.y, minCoords.z);
        edges[4] = glm::vec3(minCoords.x, minCoords.y, maxCoords.z);
        edges[5] = glm::vec3(maxCoords.x, minCoords.y, maxCoords.z);
        edges[6] = glm::vec3(minCoords.x, minCoords.y, minCoords.z);
        edges[7] = glm::vec3(maxCoords.x, minCoords.y, minCoords.z);

        this->objects = objects;
        center = getCenter();
        radius = (maxCoords - minCoords) * 0.5f;
    }

    Cell::Cell(glm::vec3 minCoords, glm::vec3 maxCoords, std::vector<GameObject*> objects){
        this->minCoords = minCoords;
        this->maxCoords = maxCoords;

        edges[0] = glm::vec3(minCoords.x, maxCoords.y, maxCoords.z);
        edges[1] = glm::vec3(maxCoords.x, maxCoords.y, maxCoords.z);
        edges[2] = glm::vec3(minCoords.x, maxCoords.y, minCoords.z);
        edges[3] = glm::vec3(maxCoords.x, maxCoords.y, minCoords.z);
        edges[4] = glm::vec3(minCoords.x, minCoords.y, maxCoords.z);
        edges[5] = glm::vec3(maxCoords.x, minCoords.y, maxCoords.z);
        edges[6] = glm::vec3(minCoords.x, minCoords.y, minCoords.z);
        edges[7] = glm::vec3(maxCoords.x, minCoords.y, minCoords.z);

        for(size_t i=0; i<objects.size(); i++){
            this->objects.insert(objects[i]);
        }
        center = getCenter();
        radius = (maxCoords - minCoords) * 0.5f;
    }

    void Cell::drawSelf(){
        Engine::debugMutex.lock();

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
       
        Engine::debugMutex.unlock();
    }

    void Cell::insert(GameObject* obj){
        
        if(children[0][0][0] == nullptr && objects.size() != Cell::maxObjects){
            
            objects.insert(obj);

        }else if(children[0][0][0] == nullptr && objects.size() == Cell::maxObjects){

            split();

            for(auto& obj :objects){
                for(int i=0; i<2; i++){
                    for(int j=0; j<2; j++){
                        for(int k=0; k<2; k++){
                            if(obj->sphereTest(children[i][j][k]->getCenter(), children[i][j][k]->radius)){

                                std::vector<glm::vec3> cellVecs = Cell::getBasicAxes();
                                std::vector<glm::vec3> objVecs = obj->transform.getBasicAxes();

                                if(Atlas::Collision::checkOBBtoOBB(cellVecs, objVecs, edges, obj->hitboxPoints)){
                                    children[i][j][k]->insert(obj);
                                }

                            }
                        }
                    }
                }
            }
        }
    }

    void Cell::split(){
        glm::vec3 center = (minCoords + maxCoords) * 0.5f;

        children[0][0][0] = new Cell(minCoords, center); // Top left forward
        children[0][0][1] = new Cell(glm::vec3(center.x, minCoords.y, minCoords.z), glm::vec3(maxCoords.x, center.y, center.z)); // Top right forward
        children[0][1][0] = new Cell(glm::vec3(minCoords.x, minCoords.y, center.z), glm::vec3(center.x, center.y, maxCoords.z)); // Top left back
        children[0][1][1] = new Cell(glm::vec3(center.x, minCoords.y, center.z), glm::vec3(maxCoords.x, center.y, maxCoords.z)); // Top right back

        // Bottom layer (y = 1)
        children[1][0][0] = new Cell(glm::vec3(minCoords.x, center.y, minCoords.z), glm::vec3(center.x, maxCoords.y, center.z)); // Bottom left forward
        children[1][0][1] = new Cell(glm::vec3(center.x, center.y, minCoords.z), glm::vec3(maxCoords.x, maxCoords.y, center.z)); // Bottom right forward
        children[1][1][0] = new Cell(glm::vec3(minCoords.x, center.y, center.z), glm::vec3(center.x, maxCoords.y, maxCoords.z)); // Bottom left back
        children[1][1][1] = new Cell(center, maxCoords); // Bottom right back
    }

    void Cell::drawAll(){
        if(children[0][0][0] == nullptr){
            drawSelf();
        }else{
            for(int i=0; i<2; i++){
                for(int j=0; j<2; j++){
                    for(int k=0; k<2; k++){
                        children[i][j][k]->drawAll();
                    }
                }
            }
        }
    }

    glm::vec3 Cell::getCenter(){
        return (maxCoords + minCoords) * 0.5f;
    }

    std::vector<glm::vec3> Cell::getBasicAxes(){
        std::vector<glm::vec3> vecs;

        vecs.push_back(glm::vec3(1.0f,0.0f,0.0f));
        vecs.push_back(Engine::worldUp);
        vecs.push_back(glm::vec3(0.0f,0.0f,1.0f));

        return vecs;
    }
}