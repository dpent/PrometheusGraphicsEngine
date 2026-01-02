#include "../headers/transform.h"

Transform::Transform(glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;
}

glm::mat4 Transform::getModelMatrix() {
    return glm::translate(glm::mat4(1.0f), position)
        * glm::toMat4(rotation)
        * glm::scale(glm::mat4(1.0f), scale);
}

std::vector<glm::vec3> Transform::getBasicAxes() {
    std::vector<glm::vec3> vecs;

    vecs.push_back(rotation * glm::vec3(1, 0, 0));
    vecs.push_back(rotation * glm::vec3(0, 1, 0));
    vecs.push_back(rotation * glm::vec3(0, 0, 1));

    return vecs;
}