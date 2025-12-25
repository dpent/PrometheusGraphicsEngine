#include "../headers/light.h"
#include "../../engine/headers/engine.h"

using namespace Prometheus::Hyperion;

namespace Prometheus::Hyperion {

    UniformBufferObject::UniformBufferObject(glm::vec4 pos, glm::vec4 color, float intensity, LightType type) {
        position = pos;
        this->color = color;
        ambientLightColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.02f);
        this->intensity = intensity;
        this->type = type;

        Engine::lights.push(new UBOContainer(this));
        Engine::recreateUBO = true;
    }

    UniformBufferObject::UniformBufferObject() {}
    UniformBufferObject::~UniformBufferObject() {}

    void UniformBufferObject::update() {
        return;
    }

    glm::mat4 UniformBufferObject::getLightVP() {
        return glm::mat4();
    }

    UBOContainer::UBOContainer(UniformBufferObject* ubo) {
        this->ubo = ubo;
    }

    UBOContainer::UBOContainer() {}

    GPULightType::GPULightType() {}

    GPULightType::GPULightType(LightType type) {
        this->type = type;
    }

    PointLight::PointLight() {}

    PointLight::PointLight(glm::vec4 pos, glm::vec4 color, float intensity, LightType type)
        : UniformBufferObject::UniformBufferObject(pos, color, intensity, type) {
        return;
    }

    void PointLight::update() {
        float time = glfwGetTime(); // or your own frame timer

        float offset = color.x * -0.8f + color.y * 0.2f + color.z * 0.8f;

        float x = 0.0f + 4.0f * cos(time + offset);
        float z = 0.0f + 4.0f * sin(time + offset);
        float y = 2.0f * sin(time * 5.0f + offset);

        position.x = x;
        position.z = z;
        position.y = y;

        //Debug::drawLine(glm::vec3(0.0f), glm::vec3(position), color);
    }

    DirectionalLight::DirectionalLight() {}

    DirectionalLight::DirectionalLight(glm::vec4 pos, glm::vec4 color, float intensity, LightType type)
        : UniformBufferObject::UniformBufferObject(pos, color, intensity, type) {
        
        Engine::shadowCreatingLights++;
        Engine::shadowCreatingLightsQueue.push(this);
        Engine::recreateShadowResources = true;
    }

    DirectionalLight::~DirectionalLight() {
        Engine::shadowCreatingLights--;
        Engine::shadowCreatingLightsQueue.popItem(this);
        Engine::recreateShadowResources = true;
    }

    void DirectionalLight::update() {
        return;
    }

    glm::mat4 DirectionalLight::getLightVP() {

        glm::mat4 view = glm::lookAt(
            glm::vec3(position.x, position.y,position.z) * 5.0f, 
            glm::vec3(0.0f), 
            glm::vec3(0.0f, 1.0f, 0.0f)
        );
        
        glm::mat4 proj = glm::orthoZO(
            -10.0f, 10.0f,
            -10.0f, 10.0f,
            0.1f, 96.0f
        );

        return proj * view;
    }
}