#version 450

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

void main() {

	 vec2 quadPos;
     vec2 rotatedPos;

    if (gl_VertexIndex == 0) quadPos = vec2(-0.5, -0.5);
    if (gl_VertexIndex == 1) quadPos = vec2( 0.5, -0.5);
    if (gl_VertexIndex == 2) quadPos = vec2(-0.5,  0.5);
    if (gl_VertexIndex == 3) quadPos = vec2( 0.5,  0.5);

    outUV = quadPos + 0.5;

    float angle = fract(sin(float(gl_InstanceIndex) * 12.9898) * 43758.5453) * 6.28318;
    float cosA = cos(angle);
    float sinA = sin(angle);

    rotatedPos.x = quadPos.x * cosA - quadPos.y * sinA;
    rotatedPos.y = quadPos.x * sinA + quadPos.y * cosA;

    vec3 camRight = vec3(pc.view[0][0], pc.view[1][0], pc.view[2][0]);
    vec3 camUp    = vec3(pc.view[0][1], pc.view[1][1], pc.view[2][1]);

    float size = 1.0;

    vec3 worldPos =
    inPosition +
    camRight * rotatedPos.x * size +
    camUp    * rotatedPos.y * size;

    gl_Position = pc.proj * pc.view * vec4(worldPos, 1.0);

    outColor = inColor;
}