#include <sstream> 
#include "../headers/mesh.h"
#include "../../debug/headers/debug.h"

using namespace Prometheus;

namespace Prometheus{
    Mesh::Mesh(){
    }

    Mesh::Mesh(std::string meshPath, std::vector<Vertex> vertices, std::vector<uint32_t> indices, 
        glm::vec3 min, glm::vec3 max){
        this->meshPath=meshPath;
        this->vertices=vertices;
        this->indices=indices;

        hitboxPoints[0] = glm::vec3(min.x, max.y, max.z);
        hitboxPoints[1] = glm::vec3(max.x, max.y, max.z);
        hitboxPoints[2] = glm::vec3(min.x, max.y, min.z);
        hitboxPoints[3] = glm::vec3(max.x, max.y, min.z);
        hitboxPoints[4] = glm::vec3(min.x, min.y, max.z);
        hitboxPoints[5] = glm::vec3(max.x, min.y, max.z);
        hitboxPoints[6] = glm::vec3(min.x, min.y, min.z);
        hitboxPoints[7] = glm::vec3(max.x, min.y, min.z);
        
        /*
        0: Top front left
        1: Top front right
        2: Top back left
        3: Top back right
        4: Bottom front left
        5: Bottom front right
        6: Bottom back left
        7: Bottom back right
        */
    }

    void Mesh::drawAABB(){

        Debug::drawLine(hitboxPoints[1], hitboxPoints[3], COLOR_MAGENTA); //TOP PART
        Debug::drawLine(hitboxPoints[1], hitboxPoints[0], COLOR_MAGENTA);
        Debug::drawLine(hitboxPoints[2], hitboxPoints[3], COLOR_MAGENTA);
        Debug::drawLine(hitboxPoints[2], hitboxPoints[0], COLOR_MAGENTA);

        Debug::drawLine(hitboxPoints[1], hitboxPoints[5], COLOR_MAGENTA); //PERPENDICULAR PARTS
        Debug::drawLine(hitboxPoints[3], hitboxPoints[7], COLOR_MAGENTA);
        Debug::drawLine(hitboxPoints[0], hitboxPoints[4], COLOR_MAGENTA);
        Debug::drawLine(hitboxPoints[2], hitboxPoints[6], COLOR_MAGENTA);

        Debug::drawLine(hitboxPoints[6], hitboxPoints[4], COLOR_MAGENTA); //BOTTOM PART
        Debug::drawLine(hitboxPoints[6], hitboxPoints[7], COLOR_MAGENTA);
        Debug::drawLine(hitboxPoints[5], hitboxPoints[4], COLOR_MAGENTA);
        Debug::drawLine(hitboxPoints[5], hitboxPoints[7], COLOR_MAGENTA);
    }

    std::string Mesh::toString(){
        std::ostringstream oss;
        oss << "Mesh { "
            << "meshPath = " << meshPath << "\n "
            << "vertexOffset = " << vertexOffset << "\n "
            << "vertices = " << vertices.size() << "\n "
            << "indexOffset = " << indexOffset << "\n "
            << "indices = " << indices.size() << " }";
            
        return oss.str();
    }

    MeshBatch::MeshBatch(std::string path){
        this->meshPath=path;
    }

    MeshBatch::MeshBatch(){
    }
}