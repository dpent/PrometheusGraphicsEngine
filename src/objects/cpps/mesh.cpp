#include <sstream> 
#include "../headers/mesh.h"

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

    InstanceInfo::InstanceInfo(glm::mat4 model, uint32_t textureIndex){
        this->modelMatrix=model;
        this->textureIndex=textureIndex;
    }

    std::string InstanceInfo::toString(){
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(3); // optional formatting
        ss<<textureIndex<<"\n";
        // glm::mat4 is column-major
        for (int row = 0; row < 4; ++row) {
            ss << "[ ";
            for (int col = 0; col < 4; ++col) {
                ss << modelMatrix[col][row];
                if (col < 3) ss << ", ";
            }
            ss << " ]\n";
        }

        return ss.str(); // <-- returns a string
    }

    MeshBatch::MeshBatch(std::string path){
        this->meshPath=path;
    }
}