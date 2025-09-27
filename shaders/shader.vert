#version 450
//Nested structures need alignment
layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat4 instanceModelMatrix;
layout(location = 7) in uint instanceTextureIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out flat uint fragTextureIndex;

void main() {
    gl_Position = pc.proj * pc.view * instanceModelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
    fragTextureIndex = instanceTextureIndex;
}