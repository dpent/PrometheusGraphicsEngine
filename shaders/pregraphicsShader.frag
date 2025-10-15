#version 450

layout(location = 0) in vec3 nearPoint;
layout(location = 1) in vec3 farPoint;
layout(location = 0) out vec4 outColor;

layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    float gridSize;
    float gridMinPixelsBetweenCells;
} camera;

// Compute world position by intersecting ray with ground plane (y = 0)
vec4 computeWorldPos() {
    vec3 rayDir = farPoint - nearPoint;
    float t = -nearPoint.y / rayDir.y;
    
    // If t < 0, ray doesn't hit the ground plane
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
    
    // Multi-scale grid for better visualization at different zoom levels
    float scale1 = camera.gridSize;
    float scale2 = camera.gridSize * 10.0;
    
    vec2 coord = worldPos.xz / scale1;
    float grid1 = getGrid(coord, scale1);
    
    coord = worldPos.xz / scale2;
    float grid2 = getGrid(coord, scale2);
    
    // Combine grids - larger grid is brighter
    float grid = max(grid1 * 0.3, grid2 * 0.6);
    
    // Distance-based fade
    float distanceToCamera = length(worldPos - camera.cameraPos);
    float fade = 1.0 - clamp(distanceToCamera / 200.0, 0.0, 1.0);
    
    // Axis lines (X = red, Z = blue)
    float axisWidth = 0.02;
    float xAxis = step(abs(worldPos.z), axisWidth) * 0.8;
    float zAxis = step(abs(worldPos.x), axisWidth) * 0.8;
    
    vec3 color = vec3(0.7412, 0.0902, 0.698) * grid;
    color = mix(color, vec3(1.0, 0.0, 0.0), xAxis); // Red X-axis
    color = mix(color, vec3(0.0, 0.0, 1.0), zAxis); // Blue Z-axis
    
    float alpha = (grid + xAxis + zAxis) * fade;
    
    // Depth calculation for proper depth testing
    vec4 clipPos = camera.projection * camera.view * vec4(worldPos, 1.0);
    gl_FragDepth = clipPos.z / clipPos.w;
    
    outColor = vec4(color, alpha);
}