#include "../headers/collision.h"


using namespace Atlas;

namespace Atlas{

    bool Collision::checkOBBtoOBB(const std::vector<glm::vec3>& vecA, const std::vector<glm::vec3>& vecB,
        const std::array<glm::vec3, 8>& vertA, const std::array<glm::vec3, 8>& vertB){
        
        std::vector<glm::vec3> allAxesTested;

        allAxesTested.reserve(15);

        allAxesTested.push_back(vecA[0]);
        allAxesTested.push_back(vecA[1]);
        allAxesTested.push_back(vecA[2]);
        allAxesTested.push_back(vecB[0]);
        allAxesTested.push_back(vecB[1]);
        allAxesTested.push_back(vecB[2]);

        for(int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                glm::vec3 cross = glm::cross(vecA[i], vecB[j]);

                float len2 = glm::dot(cross, cross);
                if (len2 < 1e-6f) continue;

                allAxesTested.push_back(glm::normalize(cross));
            }
        }

        for(glm::vec3& axis: allAxesTested){
            std::pair<float,float> projA = Collision::projectVertsAndReturnMinMax(vertA, axis);
            std::pair<float,float> projB = Collision::projectVertsAndReturnMinMax(vertB, axis);

            if (projA.second < projB.first || projB.second < projA.first) {
                return false;
            }
        }

        return true;
    }

    std::pair<float,float> Collision::projectVertsAndReturnMinMax(const std::array<glm::vec3, 8>& verts, const glm::vec3& axis){
        float minP = glm::dot(verts[0], axis);
        float maxP = minP;

        for (int i = 1; i < 8; i++) {
            float p = glm::dot(verts[i], axis);
            if (p < minP) minP = p;
            if (p > maxP) maxP = p;
        }
        return std::pair<float,float>(minP, maxP);
    }
 
}