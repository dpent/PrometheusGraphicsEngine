#version 450

layout(set = 0, binding = 0) uniform sampler linearSampler;
layout(set = 0, binding = 1) uniform texture2D image;

layout(location = 1) in vec4 inColor;
layout(location = 0) in vec2 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = texture(sampler2D(image, linearSampler), inUV);
    outColor = texColor * inColor;

}