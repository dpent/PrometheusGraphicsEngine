#pragma once
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>  
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/common.hpp>
#include <vector>

namespace Prometheus{
    struct Transform{
    public:
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;

        Transform(glm::vec3 position = glm::vec3(0.0f),
        glm::quat rotation = glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
        glm::vec3 scale = glm::vec3(1.0f));

        glm::mat4 getModelMatrix();

        std::vector<glm::vec3> getBasicAxes();
    };
}