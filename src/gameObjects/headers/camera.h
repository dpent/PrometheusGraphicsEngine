#pragma once

#include "../../core/headers/Prometheus.h"

class Camera {
public:

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 velocity;
    glm::quat orientation;

    float yaw;
    float pitch;
    float roll;
    float sensitivity;
    float fov;
    float far;
    float acceleration;
    float maxSpeed; 

    Camera(glm::vec3 position = glm::vec3(3.0f, 3.0f, 3.0f),
        glm::vec3 front = glm::vec3(-0.577382f, -0.577288f, -0.577382f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3 right = glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::quat orientation = glm::quat(1.0, 0.0, 0.0, 0.0),
        float yaw = -135.0f,
        float pitch = -35.26f,
        float roll = 0.0f,
        glm::vec3 velocity = glm::vec3(0.0f),
        float sensitivity = 0.1f,
        float fov = 45.0f,
        float far = 200.0f,
        float acceleration = 0.01f,
        float maxSpeed = 10.0f
    );

    void updateCameraVectors();

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix();

    // Getters
    const glm::vec3& getPosition() const;
    const glm::vec3& getFront() const;
    const glm::vec3& getUp() const;
    const glm::vec3& getRight() const;
    float getYaw() const;
    float getPitch() const;
    float getRoll() const;
    glm::vec3 getVelocity() const;
    float getSensitivity() const;
    float getFOV() const;
    float getFar() const;
    float getAcceleration() const;
    float getMaxSpeed() const;

    // Setters
    void setPosition(const glm::vec3& pos);
    void setFront(const glm::vec3& dir);
    void setUp(const glm::vec3& u);
    void setRight(const glm::vec3& r);
    void setYaw(float y);
    void setPitch(float p);
    void setRoll(float r);
    void setVelocity(glm::vec3 v);
    void setSensitivity(float s);
    void setFOV(float fov);
    void setFar(float far);
	void setAcceleration(float a);
	void setMaxSpeed(float ms);
};