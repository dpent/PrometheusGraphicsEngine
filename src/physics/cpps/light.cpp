#include "../headers/light.h"
#include "../../core/headers/engine.h"

Light::Light(glm::vec4 pos, glm::vec4 color, float intensity) {
    position = pos;
    this->color = color;
    ambientLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.02f);
    this->intensity = intensity;
    this->type = type;
}

Light::Light() {}
Light::~Light() {}

void Light::update() {
    return;
}

glm::mat4 Light::getLightVP() {
    return glm::mat4();
}

PointLight::PointLight() {}

PointLight::PointLight(glm::vec4 pos, glm::vec4 color, float intensity)
    : Light::Light(pos, color, intensity) {

    this->type = LIGHT_POINT;
    Engine::lights.push(this);
}

void PointLight::update() {
    float time = static_cast<float>(glfwGetTime()); // or your own frame timer

    float offset = color.x * -0.8f + color.y * 0.2f + color.z * 0.8f;

    float x = 22.0f + 8.0f * cos(time + offset);
    float z = 22.0f + 8.0f * sin(time + offset);
    //float y = 2.0f * sin(time * 5.0f + offset) + 3.0f;

    position.x = x;
    position.z = z;
    //position.y = y;

    //Debug::drawLine(glm::vec3(0.0f), glm::vec3(position), color);
}

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(glm::vec4 pos, glm::vec4 color, float intensity)
    : Light::Light(pos, color, intensity) {

    Engine::shadowCreatingLights.push(this);
    Engine::recreateShadowResources = true;
    this->type = LIGHT_DIRECTIONAL;

}

DirectionalLight::~DirectionalLight() {
    Engine::shadowCreatingLights.popItem(this);
    Engine::recreateShadowResources = true;
}

void DirectionalLight::update() {
    /*float time = static_cast<float>(glfwGetTime()); // or your own frame timer

    float offset = color.x * -0.8f + color.y * 0.0f + color.z * 0.8f;

    float x = 0.0f + 15.0f * cos(time + offset);
    float z = 0.0f + 15.0f * sin(time + offset);
    position.x = x;
    position.z = z;*/
    return;
}

glm::mat4 DirectionalLight::getLightVP() {

    glm::mat4 view = glm::lookAt(
        -glm::vec3(position.x - 10.0f, position.y - 10.0f, position.z - 10.0f), //Negative so i can get the proper image from the light perspective
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float near = 0.1f;
    float far = 150.f;

    float orthoSize = 25.0f;

    glm::mat4 proj = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        near, far
    );

    proj[1][1] *= -1;

    return proj * view;
}