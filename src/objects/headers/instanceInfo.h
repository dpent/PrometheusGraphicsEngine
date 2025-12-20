#pragma once
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include <string>
#include <sstream>
#include <iomanip>

namespace Prometheus
{
    
    struct InstanceInfo{
        glm::mat4 modelMatrix;
        alignas(16) uint32_t textureIndex;
    
        InstanceInfo(glm::mat4 model, uint32_t textureIndex);
        InstanceInfo();
    
        std::string toString();
    };
}
