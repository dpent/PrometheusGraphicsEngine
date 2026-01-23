#version 450

struct ObjectData {
    mat4 model;
    uint textureIndex;
    uint padding[3];  // 12 bytes

    uint hasNormal;   // offset 80
    uint padding1[3]; // offset 84
};

layout(std430, binding = 1) readonly buffer instanceData
{
    ObjectData instances[];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec4 tangent;

layout(push_constant) uniform lightVP {
    mat4 VP;
    uint instanceIndex;
} light;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
    ObjectData obj = instances[light.instanceIndex];
	gl_Position = light.VP * obj.model * vec4(inPosition, 1.0);
}