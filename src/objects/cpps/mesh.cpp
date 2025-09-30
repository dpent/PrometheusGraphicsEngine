#include <sstream> 
#include "../headers/mesh.h"

using namespace Prometheus;

namespace Prometheus{
    Mesh::Mesh(){

    }

    Mesh::Mesh(std::string meshPath, std::vector<Vertex> vertices, std::vector<uint32_t> indices){
        this->meshPath=meshPath;
        this->vertices=vertices;
        this->indices=indices;
    }

    std::string Mesh::toString(){
        std::ostringstream oss;
        oss << "Mesh { "
            << "vertexOffset=" << vertexOffset << ", "
            << "indexOffset=" << indexOffset << ", "
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