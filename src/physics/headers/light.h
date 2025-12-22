#pragma once
#include <glm/glm.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Prometheus::Hyperion {
    enum LightType : uint8_t {
        LIGHT_DIRECTIONAL = 0,
        LIGHT_POINT = 1
    };

    struct GPULightType {
    public:
        LightType type;

        GPULightType(LightType type);
        GPULightType();
    };

    struct UniformBufferObject {
    public:

        glm::vec4 position;
        glm::vec4 color;
        glm::vec4 ambientLightColor;
        float intensity;
        LightType type;

        UniformBufferObject(glm::vec4 pos, glm::vec4 color, float intensity, LightType type);
        UniformBufferObject();

        ~UniformBufferObject();

        virtual void update();
    };

    struct PointLight : public UniformBufferObject {

        PointLight();
        PointLight(glm::vec4 pos, glm::vec4 color, float intensity, LightType type);

        void update() override;
    };

    struct DirectionalLight : public UniformBufferObject {

        DirectionalLight();
        DirectionalLight(glm::vec4 pos, glm::vec4 color, float intensity, LightType type);
        ~DirectionalLight();
        void update() override;
    };

    struct UBOData {
        glm::vec4 positions[128];
        glm::vec4 colors[128];
        glm::vec4 ambientLightColors[128];
        glm::vec4 intensities[128];
        alignas(16) uint64_t lightCount;
        alignas(16) uint32_t types[128 / 4] = {};
    };

    class UBOContainer {
    public:
        UniformBufferObject* ubo;
        UBOContainer* next;
        UBOContainer* prev;

        UBOContainer(UniformBufferObject* ubo);
        UBOContainer();
    };
}