#include "../headers/camera.h"
#include "../../core/headers/engine.h"

Camera::Camera(glm::vec3 position, glm::vec3 front,
    glm::vec3 up, glm::vec3 right, glm::quat orientation,
    float yaw, float pitch, float roll, glm::vec3 velocity,
    float sensitivity, float fov, float far, float acceleration,
    float maxSpeed)
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
    this->far = far;
    this->acceleration = acceleration;
	this->maxSpeed = maxSpeed;
}

void Camera::updateCameraVectors() {

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = glm::normalize(front);

    right = glm::normalize(glm::cross(this->front, Engine::WORLD_UP));
    up = glm::normalize(glm::cross(right, this->front));
}

glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() {

    glm::mat4 proj = glm::perspective(
        glm::radians(fov),
        (float)Engine::swapChainInfo.extent.width /
        (float)Engine::swapChainInfo.extent.height, 0.01f, far
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
glm::vec3 Camera::getVelocity() const { return velocity; }
float Camera::getSensitivity() const { return sensitivity; }
float Camera::getFOV() const { return fov; }
float Camera::getFar() const { return far; }
float Camera::getAcceleration() const { return acceleration; }
float Camera::getMaxSpeed() const { return maxSpeed; }

// --- Setters ---
void Camera::setPosition(const glm::vec3& pos) { position = pos; }
void Camera::setFront(const glm::vec3& dir) { front = glm::normalize(dir); }
void Camera::setUp(const glm::vec3& u) { up = glm::normalize(u); }
void Camera::setRight(const glm::vec3& r) { right = glm::normalize(r); }
void Camera::setYaw(float y) { yaw = y; }
void Camera::setPitch(float p) { pitch = p; }
void Camera::setRoll(float s) { roll = s; }
void Camera::setVelocity(glm::vec3 v) { velocity = v; }
void Camera::setSensitivity(float s) { sensitivity = s; }
void Camera::setFOV(float fov) { this->fov = fov; }
void Camera::setFar(float far) { this->far = far; }
void Camera::setAcceleration(float a) { acceleration = a; }
void Camera::setMaxSpeed(float ms) { maxSpeed = ms; }