#version 450
#extension GL_EXT_nonuniform_qualifier : enable

struct ObjectData {
    mat4 model;
    uint textureIndex;
    uint padding[3];// 12 bytes padding to align to 16 bytes
};

layout(std430, binding = 1) readonly buffer instanceData
{
    ObjectData instances[];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
    uint instanceIndex;
} pc;

layout(location = 0) out vec3 vertColor;
layout(location = 1) out uint objectIndex;
layout(location = 2) out vec2 texCoord;
layout(location = 3) out uint texIndex;
layout(location = 4) out vec3 worldPos;
layout(location = 5) out vec3 worldNormal;

void main() {

    ObjectData obj = instances[pc.instanceIndex];

	gl_Position = pc.proj * pc.view * obj.model * vec4(inPosition, 1.0);

    objectIndex = pc.instanceIndex;
    texIndex = obj.textureIndex;
    vertColor = inColor;
    texCoord = inTexCoord;
    worldPos = (obj.model * vec4(inPosition, 1.0)).xyz;
    worldNormal = normalize(mat3(obj.model) * inNormal);
}