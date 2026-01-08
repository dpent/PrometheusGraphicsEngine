#pragma once
#include "../../core/headers/Prometheus.h"

enum LightType : uint8_t {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1
};

struct Light {
public:

    glm::vec4 position; //For directional light this is the direction
    glm::vec4 color;
    glm::vec4 ambientLightColor;
    float intensity;
    LightType type;

    Light* next;
    Light* prev;

    Light(glm::vec4 pos, glm::vec4 color, float intensity);
    Light();

    ~Light();

    virtual void update();
    virtual glm::mat4 getLightVP();
};

struct PointLight : public Light {

    PointLight();
    PointLight(glm::vec4 pos, glm::vec4 color, float intensity);

    void update() override;
};

struct DirectionalLight : public Light {

    DirectionalLight();
    DirectionalLight(glm::vec4 pos, glm::vec4 color, float intensity);
    ~DirectionalLight();
    void update() override;
    glm::mat4 getLightVP() override;
};

struct UBOData {

};

struct LightUBOData : public UBOData{
    glm::vec4 positions[128];
    glm::vec4 colors[128];
    glm::vec4 ambientLightColors[128];
    glm::vec4 intensities[128];
    glm::mat4 lightMVPs[128];
    alignas(16) uint64_t lightCount;
    alignas(16) uint32_t types[128 / 4] = {};
};