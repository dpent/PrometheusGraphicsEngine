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