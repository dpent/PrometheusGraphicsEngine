#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec3 nearPoint;
layout(location = 1) out vec3 farPoint;

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float gridSize;
    
} camera;

vec3 unprojectPoint(float x, float y, float z) {
    mat4 viewInv = inverse(camera.view);
    mat4 projInv = inverse(camera.projection);
    vec4 unprojected = viewInv * projInv * vec4(x, y, z, 1.0);
    return unprojected.xyz / unprojected.w;
}

void main() {
    // Fullscreen triangle trick
    vec2 positions[3] = vec2[](
        vec2(-1, -1),
        vec2( 3, -1),
        vec2(-1,  3)
    );
    
    vec2 pos = positions[gl_VertexIndex];
    gl_Position = vec4(pos, 0.0, 1.0);
    
    // Unproject to get near and far points in world space
    nearPoint = unprojectPoint(pos.x, pos.y, 0.0); // Near plane
    farPoint = unprojectPoint(pos.x, pos.y, 1.0);  // Far plane
}