#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
} pc;

layout(set = 0, binding = 0) uniform Lights {
    vec4 positions[128];
    vec4 colors[128];
    vec4 ambientColors[128];
    vec4 intensities[128];
    uint lightCount;
} lightsUBO;

layout(location = 0) out vec2 fragQuadPos;
layout(location = 1) out vec3 color;

void main() {
    uint lightIndex = gl_InstanceIndex;

    vec2 quadCorner = vec2(
        (gl_VertexIndex & 1) * 2.0 - 1.0,
        (gl_VertexIndex >> 1) * 2.0 - 1.0
    );

    vec3 cameraRight = vec3(pc.view[0][0], pc.view[1][0], pc.view[2][0]);
    vec3 cameraUp = vec3(pc.view[0][1], pc.view[1][1], pc.view[2][1]);
    
    // Light properties
    vec3 lightPos = lightsUBO.positions[lightIndex].xyz;
    float lightSize = (lightsUBO.intensities[lightIndex].x) / 10.0;
    
    // Billboard the quad to face camera
    vec3 worldPos = lightPos 
        + cameraRight * quadCorner.x * lightSize
        + cameraUp * quadCorner.y * lightSize;
    
    gl_Position = pc.proj * pc.view * vec4(worldPos, 1.0);
    
    fragQuadPos = quadCorner * 8;
    color = lightsUBO.colors[lightIndex].xyz * lightsUBO.colors[lightIndex].w * lightsUBO.intensities[lightIndex].x;
}