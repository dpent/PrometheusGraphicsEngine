#version 450
#extension GL_KHR_vulkan_glsl : enable
//Nested structures need alignment
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat uint fragTextureIndex;
layout(location = 3) in vec3 fragWorldNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textures[64];

layout(set = 0, binding = 0) uniform Lights {
    vec4 positions[128]; // or vec4 array for your lights
    vec4 colors[128];
    vec4 ambientColors[128];
    vec4 intensities[128];
    uint lightCount;
    uint types[128/4];
} lightsUBO;

void main() {
    vec4 texColor = vec4((texture(textures[fragTextureIndex], fragTexCoord)).xyz * 0.5, 1.0);
    vec3 totalLight = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);

    for (uint i = 0; i < lightsUBO.lightCount; i++) {

    
        uint idx     = i / 4u;  // which uint
        uint bytePos = i % 4u;  // which byte inside it
        uint packed = lightsUBO.types[idx];
        uint type   = (packed >> (8u * bytePos)) & 0xFFu;

        if(type == 0u){ //DIRECTIONAL = 0
            vec3 directionToLight = lightsUBO.positions[i].xyz - fragWorldPos;
            float distance2 = dot(directionToLight, directionToLight);
            float attenuation = 1.0 / max(distance2, 0.001); // avoid divide by zero

            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;

            vec3 diffuseLight = lightColor * max(dot(normalize(fragWorldNormal), normalize(directionToLight)), 0);

            totalLight += diffuseLight * attenuation;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;

        }else if(type == 1u){ //POINT LIGHT = 1
            vec3 directionToLight = lightsUBO.positions[i].xyz - vec3(0.0);

            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;
            lightColor = lightColor/10.0;

            vec3 diffuseLight = lightColor * max(dot(normalize(fragWorldNormal), normalize(directionToLight)), 0);

            totalLight += diffuseLight;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;
        }
    }

    vec4 lightColor = vec4(totalAmbient + totalLight, 1.0) / lightsUBO.lightCount;

    outColor = lightColor + texColor;
}