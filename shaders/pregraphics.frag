#version 450

layout(location = 0) in vec3 nearPoint;
layout(location = 1) in vec3 farPoint;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float gridSize;
} camera;

vec4 computeWorldPos() {
    vec3 rayDir = farPoint - nearPoint;

    float t = -nearPoint.y / rayDir.y;
    
    if (t < 0.0) {
        discard;
    }
    
    vec3 worldPos = nearPoint + t * rayDir;
    return vec4(worldPos, t);
}

float getGrid(vec2 coord, float scale) {
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
    float line = min(grid.x, grid.y);
    return 1.0 - min(line, 1.0);
}

void main() {
    vec4 worldPosData = computeWorldPos();
    vec3 worldPos = worldPosData.xyz;
    float t = worldPosData.w;
    
    float scale1 = camera.gridSize;
    float scale2 = camera.gridSize * 10.0;
    
    vec2 coord = worldPos.xz / scale1;
    float grid1 = getGrid(coord, scale1);
    
    coord = worldPos.xz / scale2;
    float grid2 = getGrid(coord, scale2);
    
    float grid = max(grid1 * 0.3, grid2 * 0.6);
    
    float distanceToCamera = length(worldPos - camera.cameraPos);
    float fade = 1.0 - clamp(distanceToCamera / 200.0, 0.0, 1.0);
    
    float axisWidth = 0.02;
    float xAxis = step(abs(worldPos.z), axisWidth) * 0.8;
    float zAxis = step(abs(worldPos.x), axisWidth) * 0.8;
    
    vec3 color = vec3(0.2039, 0.1765, 0.2039) * grid;
    color = mix(color, vec3(1.0, 0.0, 0.0), xAxis);
    color = mix(color, vec3(0.0, 0.0, 1.0), zAxis);
    
    float alpha = clamp(grid + xAxis + zAxis, 0.0, 1.0) * fade;
    
    vec4 clipPos = camera.projection * camera.view * vec4(worldPos, 1.0);
    gl_FragDepth = clipPos.z / clipPos.w;
    
    outColor = vec4(color, alpha);
}