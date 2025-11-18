#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  

namespace Prometheus
{
    class Debug{
        public:

        static void drawLine(glm::vec3 startPos, glm::vec3 endPos, glm::vec3 color);
        static void drawLine(glm::vec2 startPos, glm::vec2 endPos, glm::vec3 color);
    };
}


