#include "../headers/transform.h"

using namespace Prometheus;

namespace Prometheus{
    Transform::Transform(glm::vec3 position,glm::quat rotation,glm::vec3 scale)
    {
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
    }

    glm::mat4 Transform::getModelMatrix(){
        return glm::translate(glm::mat4(1.0f), position)
             * glm::toMat4(rotation)
             * glm::scale(glm::mat4(1.0f), scale);
    }
}