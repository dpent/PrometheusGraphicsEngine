#include "../headers/instanceInfo.h"

using namespace Prometheus;

namespace Prometheus{

    InstanceInfo::InstanceInfo(glm::mat4 model, uint32_t textureIndex){
        this->modelMatrix=model;
        this->textureIndex=textureIndex;
    }

    InstanceInfo::InstanceInfo(){}

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
}