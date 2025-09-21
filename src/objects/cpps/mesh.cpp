#include "../../engine/headers/engine.h"
#include <sstream> 

using namespace Prometheus;

namespace Prometheus{
    Mesh::Mesh(){

    }

    Mesh::Mesh(uint32_t vertexOffset, uint32_t vertexCount, uint32_t indexOffset, uint32_t indexCount){
        this->vertexOffset=vertexOffset;
        this->vertexCount=vertexCount;
        this->indexCount=indexCount;
        this->indexOffset=indexOffset;
    }

    std::string Mesh::toString(){
        std::ostringstream oss;
        oss << "Mesh { "
            << "vertexOffset=" << vertexOffset << ", "
            << "vertexCount=" << vertexCount << ", "
            << "indexOffset=" << indexOffset << ", "
            << "indexCount=" << indexCount << ", "
            << "meshPath=\"" << meshPath << "\" }";
        return oss.str();
    }
}