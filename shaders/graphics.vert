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
    vec4 intensities[128];
    uint  lightCount;
} lightsUBO;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat4 instanceModelMatrix;
layout(location = 7) in uint instanceTextureIndex;
layout(location = 8) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat uint fragTextureIndex;
layout(location = 3) out vec3 fragWorldNormal;
layout(location = 4) out vec3 fragWorldPos;

void main() {
    vec4 worldPos = instanceModelMatrix * vec4(inPosition, 1.0);
    gl_Position = pc.proj * pc.view * instanceModelMatrix * vec4(inPosition, 1.0);

    fragWorldNormal = normalize(mat3(instanceModelMatrix) * inNormal);
    fragWorldPos = worldPos.xyz;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTextureIndex = instanceTextureIndex;
}