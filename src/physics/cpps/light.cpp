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

void Light::placeDataInUBO(size_t& i) {
    Engine::lightData.positions[i] = position;
    Engine::lightData.colors[i] = color;
    Engine::lightData.ambientLightColors[i] = ambientLightColor;
    Engine::lightData.intensities[i] = glm::vec4(intensity);
    Engine::lightData.lightVPs[i] = getLightVP();

    size_t idx = i / 4;        // which uint32_t
    uint32_t bytePos = i % 4;    // which byte in that uint32_t
    uint32_t shift = 8 * (3 - bytePos);
    uint32_t mask = 0xFFu << shift;

    uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;      // which uvec4
    uint32_t  uvecElement = idx % 4;    // which uint inside that uvec4

    Engine::lightData.types[uvecIndex][uvecElement] =
        (Engine::lightData.types[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)type << shift);
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

    float x = 0.0f + 8.0f * cos(time + offset);
    float z = 22.0f + 8.0f * sin(time + offset);
    //float y = 2.0f * sin(time * 5.0f + offset) + 3.0f;

    position.x = x;
    position.z = z;
    //position.y = y;

    //Debug::drawLine(glm::vec3(0.0f), glm::vec3(position), color);
}

PointLight::~PointLight() {
    Engine::lights.popItem(this);
}

void PointLight::placeDataInUBO(size_t& i) {

    Engine::lightData.positions[i] = position;
    Engine::lightData.colors[i] = color;
    Engine::lightData.ambientLightColors[i] = ambientLightColor;
    Engine::lightData.intensities[i] = glm::vec4(intensity);
    Engine::lightData.lightVPs[i] = getLightVP();

    size_t idx = i / 4;
    uint32_t bytePos = i % 4;
    uint32_t shift = 8 * (3 - bytePos);
    uint32_t mask = 0xFFu << shift;

    uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;
    uint32_t  uvecElement = idx % 4;

    Engine::lightData.types[uvecIndex][uvecElement] =
        (Engine::lightData.types[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)type << shift);
}

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(glm::vec4 dir, glm::vec4 color, float intensity)
    : ShadowLight::ShadowLight(-dir, dir, color, intensity) {

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
        glm::vec3(position) * 2.0f, //Negative so i can get the proper image from the light perspective
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float near = 0.1f;
    float far = 150.f;

    float orthoSize = 25.0f;

    glm::mat4 proj = glm::orthoRH_ZO(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        near, far
    );

    proj[1][1] *= -1;

    return proj * view;
}

void DirectionalLight::placeDataInUBO(size_t& i) {

    Engine::shadowLightData.positions[i] = position;
    Engine::shadowLightData.directions[i] = direction;
    Engine::shadowLightData.colors[i] = color;
    Engine::shadowLightData.ambientLightColors[i] = ambientLightColor;
    Engine::shadowLightData.intensities[i] = glm::vec4(intensity);
    Engine::shadowLightData.lightVPs[i] = getLightVP();

    size_t  idx = i / 4;        // which uint32_t
    uint32_t bytePos = i % 4;    // which byte in that uint32_t
    uint32_t shift = 8 * (3 - bytePos);
    uint32_t mask = 0xFFu << shift;

    uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;      // which uvec4
    uint32_t  uvecElement = idx % 4;    // which uint inside that uvec4

    Engine::shadowLightData.types[uvecIndex][uvecElement] =
        (Engine::shadowLightData.types[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)type << shift);

    Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] =
        (Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)i << shift);
}

SpotLight::SpotLight(){}

SpotLight::SpotLight(glm::vec4 pos, glm::vec4 dir, glm::vec4 color, float intensity, float coneAngle, float falloffAngle)
    : ShadowLight::ShadowLight(pos, dir, color, intensity) {

        Engine::shadowCreatingLights.push(this);
        Engine::recreateShadowResources = true;
        this->type = LIGHT_SPOT;
        this->coneAngleCos = cos(glm::radians(coneAngle));
        this->coneAngle = coneAngle;
        this->falloffAngleCos = cos(glm::radians(falloffAngle));
}

void SpotLight::update() {

    direction.x = cos((float)glfwGetTime());
    direction.z = sin((float)glfwGetTime());
    direction.y = -5.0f;
    direction = glm::normalize(direction);

    return;
}

glm::mat4 SpotLight::getLightVP() {

    glm::mat4 view = glm::lookAt(
        glm::vec3(position),
        glm::vec3(position + direction),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    float near = 0.1f;
    float far = 90.f;

    glm::mat4 proj = glm::perspective(
        glm::radians(coneAngle * 20.0f),
        1.0f,
        near,
        far
    );

    proj[1][1] *= -1;

    return proj * view;
}

SpotLight::~SpotLight() {
    Engine::shadowCreatingLights.popItem(this);
    Engine::recreateShadowResources = true;
}

void SpotLight::placeDataInUBO(size_t& i) {

    Engine::shadowLightData.positions[i] = position;
    Engine::shadowLightData.positions[i].w = falloffAngleCos;
    Engine::shadowLightData.directions[i] = direction;
    Engine::shadowLightData.directions[i].w = coneAngleCos;
    Engine::shadowLightData.colors[i] = color;
    Engine::shadowLightData.ambientLightColors[i] = ambientLightColor;
    Engine::shadowLightData.intensities[i] = glm::vec4(intensity);
    Engine::shadowLightData.lightVPs[i] = getLightVP();

    size_t  idx = i / 4;        // which uint32_t
    uint32_t bytePos = i % 4;    // which byte in that uint32_t
    uint32_t shift = 8 * (3 - bytePos);
    uint32_t mask = 0xFFu << shift;

    uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;      // which uvec4
    uint32_t  uvecElement = idx % 4;    // which uint inside that uvec4

    Engine::shadowLightData.types[uvecIndex][uvecElement] =
        (Engine::shadowLightData.types[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)type << shift);

    Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] =
        (Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)i << shift);
}

ShadowLight::ShadowLight() {

}

ShadowLight::ShadowLight(glm::vec4 pos, glm::vec4 dir, glm::vec4 color, float intensity)
    : Light::Light(pos, color, intensity){

    direction = glm::normalize(dir);
}

void ShadowLight::update() {
    return;
}

glm::mat4 ShadowLight::getLightVP() {
    return glm::mat4(1.0f);
}

ShadowLight::~ShadowLight(){}

void ShadowLight::placeDataInUBO(size_t& i) {

    Engine::shadowLightData.positions[i] = position;
    Engine::shadowLightData.directions[i] = direction;
    Engine::shadowLightData.colors[i] = color;
    Engine::shadowLightData.ambientLightColors[i] = ambientLightColor;
    Engine::shadowLightData.intensities[i] = glm::vec4(intensity);
    Engine::shadowLightData.lightVPs[i] = getLightVP();

    size_t  idx = i / 4;        // which uint32_t
    uint32_t bytePos = i % 4;    // which byte in that uint32_t
    uint32_t shift = 8 * (3 - bytePos);
    uint32_t mask = 0xFFu << shift;

    uint32_t  uvecIndex = static_cast<uint32_t>(idx) / 4;      // which uvec4
    uint32_t  uvecElement = idx % 4;    // which uint inside that uvec4

    Engine::shadowLightData.types[uvecIndex][uvecElement] =
        (Engine::shadowLightData.types[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)type << shift);

    Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] =
        (Engine::shadowLightData.shadowMapIndices[uvecIndex][uvecElement] & ~mask) |
        ((uint32_t)i << shift);
}