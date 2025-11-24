#version 450

layout(location = 0) in vec2 fragQuadPos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 outColor;

void main() {
    // Exponential falloff for stronger glow
    float dist = length(fragQuadPos);
    float intensity = 1.0 - exp(dist)/2.0;
    float colorBoost = smoothstep(1.0, 0.0, dist * 2.0) * intensity;
    
    if (intensity < 0.01) discard;
    
    outColor = vec4(color + colorBoost, smoothstep(1.0, 0.0, dist * 2.0));
}
