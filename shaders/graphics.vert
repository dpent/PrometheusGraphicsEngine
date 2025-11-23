#version 450
//Nested structures need alignment
layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
} pc;

layout(set = 0, binding = 0) uniform Lights {
    vec4 positions[128]; // or vec4 array for your lights
    vec4 colors[128];
    vec4 ambientColors[128];
    float intensities[128];
    uint  lightCount;
} lightsUBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat4 instanceModelMatrix;
layout(location = 7) in uint instanceTextureIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat uint fragTextureIndex;

void main() {
    vec4 worldPos = instanceModelMatrix * vec4(inPosition, 1.0);
    gl_Position = pc.proj * pc.view * instanceModelMatrix * vec4(inPosition, 1.0);

    vec3 directionToLight = lightsUBO.positions[0].xyz - worldPos.xyz;
    float attenuation = 1.0/dot(directionToLight,directionToLight);

    vec3 lightColor = lightsUBO.colors[0].xyz * lightsUBO.colors[0].w;
    vec3 ambientLight = lightsUBO.ambientColors[0].xyz * lightsUBO.ambientColors[0].w;


    fragColor = (vec3(1.0) * attenuation) * lightsUBO.intensities[0];
    fragTexCoord = inTexCoord;
    fragTextureIndex = instanceTextureIndex;
}