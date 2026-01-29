#pragma once
#include "../../core/headers/Prometheus.h"

enum LightType : uint8_t {
    LIGHT_DIRECTIONAL = 0,
    LIGHT_POINT = 1,
    LIGHT_SPOT = 2
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

    virtual void placeDataInUBO(size_t& i);

    ~Light();

    virtual void update();
    virtual glm::mat4 getLightVP();
};

struct ShadowLight : public Light {

    glm::vec4 direction;

    ShadowLight* next;
    ShadowLight* prev;

    ShadowLight(glm::vec4 pos, glm::vec4 direction, glm::vec4 color, float intensity);
    ShadowLight();

    ~ShadowLight();

    void placeDataInUBO(size_t& i) override;
    void update();
    glm::mat4 getLightVP();
};

struct PointLight : public Light {

    PointLight();
    PointLight(glm::vec4 pos, glm::vec4 color, float intensity);
    ~PointLight();

    void placeDataInUBO(size_t& i);

    void update() override;
};

struct DirectionalLight : public ShadowLight { //POSITION HERE IS DIRECTION

    DirectionalLight();
    DirectionalLight(glm::vec4 dir, glm::vec4 color, float intensity);
    ~DirectionalLight();
    void placeDataInUBO(size_t& i) override;
    void update() override;
    glm::mat4 getLightVP() override;
};

struct SpotLight : public ShadowLight {

    float coneAngle;
    float coneAngleCos;
    float falloffAngleCos;

    SpotLight();
    SpotLight(glm::vec4 pos, glm::vec4 dir, glm::vec4 color, float intensity, float coneAngle, float falloffAngle);
    ~SpotLight();
    void placeDataInUBO(size_t& i) override;
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
    glm::mat4 lightVPs[128];
    alignas(16) uint32_t lightCount;
    alignas(16) glm::uvec4 types[128 / 16] = {};
};

static_assert(sizeof(LightUBOData) % 16 == 0, "UBO size must be multiple of 16");

struct ShadowLightUBOData : public UBOData {
    glm::vec4 positions[64];
    glm::vec4 directions[64];
    glm::vec4 colors[64];
    glm::vec4 ambientLightColors[64];
    glm::vec4 intensities[64];
    glm::mat4 lightVPs[64];
    alignas(16) uint32_t lightCount; 
    alignas(16) glm::uvec4 types[64 / 16] = {};
    alignas(16) glm::uvec4 shadowMapIndices[64 / 16] = {};
};

static_assert(sizeof(ShadowLightUBOData) % 16 == 0, "UBO size must be multiple of 16");