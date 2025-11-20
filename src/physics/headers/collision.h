#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>  
#include <vector>
#include <array>

namespace Atlas{

    class Collision{
        public:

        static bool checkOBBtoOBB(const std::vector<glm::vec3>& vecA, const std::vector<glm::vec3>& vecB,
            const std::array<glm::vec3, 8>& vertA, const std::array<glm::vec3, 8>& vertB);

        private:
        static std::pair<float,float> projectVertsAndReturnMinMax(const std::array<glm::vec3, 8>& verts, const glm::vec3& axis);
    };
}