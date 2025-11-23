#version 450
//Nested structures need alignment
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat uint fragTextureIndex;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textures[64];

void main() {
    vec4 texColor = texture(textures[fragTextureIndex], fragTexCoord);
    outColor = vec4(fragColor, 1.0) * texColor;
}