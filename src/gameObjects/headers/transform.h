#pragma once

#include "../../core/headers/Prometheus.h"

struct Transform {
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