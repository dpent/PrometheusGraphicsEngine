#version 450
#extension GL_EXT_nonuniform_qualifier : enable


layout(location = 0) in vec3 vertColor;
layout(location = 1) in flat uint objectIndex;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in flat uint texIndex;

layout(set = 0, binding = 2) uniform texture2D textures[];

layout(set = 0, binding = 0) uniform sampler linearSampler;


layout(location = 0) out vec4 outColor;

void main(){
	outColor = texture(sampler2D(textures[texIndex], linearSampler), texCoord);
}