#include "../headers/camera.h"
#include "../../engine/headers/engine.h"

using namespace Prometheus;


namespace Prometheus{

    Camera::Camera(glm::vec3 position, glm::vec3 front,
        glm::vec3 up, glm::vec3 right, glm::quat orientation, 
        float yaw, float pitch, float roll, float velocity,
        float sensitivity, float fov)
    {
        this->position = position;
        this->front = front;
        this->up = up;
        this->right = right;
        this->orientation = orientation;
        this->yaw = yaw;
        this->pitch = pitch;   
        this->roll = roll; 
        this->velocity = velocity;
        this->sensitivity = sensitivity;
        this->fov = fov;
    }

    void Camera::updateCameraVectors(){

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->front = glm::normalize(front);

        glm::mat4 roll_mat = glm::rotate(glm::mat4(1.0f), glm::radians(roll), this->front);

        right = glm::normalize(glm::cross(front, Engine::worldUp));//glm::mat3(roll_mat) * right;
        up = glm::mat3(roll_mat) * glm::normalize(glm::cross(right, this->front));
    }

    glm::mat4 Camera::getViewMatrix(){
        return glm::lookAt(position,position + front,up);
    }

    glm::mat4 Camera::getProjectionMatrix(){

        glm::mat4 proj = glm::perspective(
            glm::radians(fov),
            Engine::swapChainExtent.width / 
            (float) Engine::swapChainExtent.height, 0.01f, 200.0f
            );

        proj[1][1] *= -1;

        return proj;
    }

    // --- Getters ---
    const glm::vec3& Camera::getPosition() const { return position; }
    const glm::vec3& Camera::getFront() const { return front; }
    const glm::vec3& Camera::getUp() const { return up; }
    const glm::vec3& Camera::getRight() const { return right; }
    float Camera::getYaw() const { return yaw; }
    float Camera::getPitch() const { return pitch; }
    float Camera::getRoll() const { return roll; }
    float Camera::getVelocity() const { return velocity; }
    float Camera::getSensitivity() const { return sensitivity; }
    float Camera::getFOV() const { return fov; }

    // --- Setters ---
    void Camera::setPosition(const glm::vec3& pos) { position = pos; }
    void Camera::setFront(const glm::vec3& dir) { front = glm::normalize(dir); }
    void Camera::setUp(const glm::vec3& u) { up = glm::normalize(u); }
    void Camera::setRight(const glm::vec3& r) { right = glm::normalize(r); }
    void Camera::setYaw(float y) { yaw = y;}
    void Camera::setPitch(float p) { pitch = p;}
    void Camera::setRoll(float s) { roll = s;}
    void Camera::setVelocity(float v) { velocity = std::max(v,0.0f);}
    void Camera::setSensitivity(float s) { sensitivity = s;}
    void Camera::setFOV(float fov) { this->fov = fov;}
}