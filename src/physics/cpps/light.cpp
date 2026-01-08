#include "../headers/light.h"
#include "../../core/headers/engine.h"

Light::Light(glm::vec4 pos, glm::vec4 color, float intensity) {
    position = pos;
    this->color = color;
    ambientLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.02f);
    this->intensity = intensity;
    this->type = type;

    Engine::lights.push(this);
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


}

DirectionalLight::~DirectionalLight() {
    Engine::shadowCreatingLights.popItem(this);
    Engine::recreateShadowResources = true;
}

void DirectionalLight::update() {
    float time = static_cast<float>(glfwGetTime()); // or your own frame timer

    float offset = color.x * -0.8f + color.y * 0.0f + color.z * 0.8f;

    float x = 0.0f + 10.0f * cos(time + offset);
    float z = 0.0f + 10.0f * sin(time + offset);
    position.x = x;
    position.z = z;
}

glm::mat4 DirectionalLight::getLightVP() {

    glm::mat4 view = glm::lookAt(
        glm::vec3(position.x, position.y, position.z),
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 proj = glm::orthoZO(
        -10.0f, 10.0f,
        -15.0f, 15.0f,
        0.1f, 96.0f
    );

    return proj * view;
}