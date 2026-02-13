#version 450

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 worldPos;
layout(location = 1) out vec3 outColor;

void main() {

	gl_Position = pc.proj * pc.view * vec4(inPosition, 1.0);
    gl_PointSize = 5.0;

    worldPos = (pc.proj * pc.view * vec4(inPosition, 1.0)).xyz;
    outColor = inColor;
}