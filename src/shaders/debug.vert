#version 450

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 proj;
    uint instanceIndex;
} pc;

struct Line {
    vec4 start;
    vec4 end;
    vec4 color;
};

layout(std430, binding = 0) readonly buffer lines
{
    Line instances[];
};


layout(location = 0) in float inT;

layout(location = 0) out vec3 fragColor;

void main() {
    Line line = instances[gl_InstanceIndex];
    vec3 pos  = mix(line.start.xyz, line.end.xyz, inT);
    gl_Position = pc.proj * pc.view * vec4(pos, 1.0);
    fragColor = line.color.xyz;
}