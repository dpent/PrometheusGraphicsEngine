#version 450


layout(push_constant) uniform CameraObject {
    mat4 view;
    mat4 projection;
    
} camera;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 color;

void main() {
    gl_Position = camera.projection * camera.view * vec4(inPosition, 1.0);
    gl_PointSize = 5.0;
    color = inPosition;
}