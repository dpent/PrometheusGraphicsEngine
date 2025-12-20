#pragma once
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES //REMEMBER THIS IS SUPPOSED TO ALIGN EVERYTHING
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Prometheus{
    class Camera{
    public:
        glm::vec3 position;
        glm::vec3 front;
        glm::vec3 up;
        glm::vec3 right;
        glm::quat orientation;

        float yaw;
        float pitch;
        float roll;
        float velocity;
        float sensitivity;
        float fov;
        float far;

        Camera(glm::vec3 position = glm::vec3(0.0f,0.0f,-10.0f), 
        glm::vec3 front = glm::vec3(0.0f,0.0f,1.0f),
        glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f),
        glm::vec3 right = glm::vec3(-1.0f,0.0f,0.0f),
        glm::quat orientation = glm::quat(1.0, 0.0, 0.0, 0.0),
        float yaw = 0.0f,
        float pitch = 0.0f,
        float roll = 0.0f,
        float velocity = 0.1f,
        float sensitivity = 0.1f,
        float fov = 45.0f,
        float far = 200.0f);

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
        float getVelocity() const;
        float getSensitivity() const;
        float getFOV() const;
        float getFar() const;

        // Setters
        void setPosition(const glm::vec3& pos);
        void setFront(const glm::vec3& dir);
        void setUp(const glm::vec3& u);
        void setRight(const glm::vec3& r);
        void setYaw(float y);
        void setPitch(float p);
        void setRoll(float r);
        void setVelocity(float v);
        void setSensitivity(float s);
        void setFOV(float fov);
        void setFar(float far);
    };
}