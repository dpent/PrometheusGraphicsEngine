#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in mat4 instanceModelMatrix;
layout(location = 7) in uint instanceTextureIndex;
layout(location = 8) in vec3 inNormal;

layout(push_constant) uniform lightVP {
    mat4 VP;
    
} light;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	gl_Position = light.VP * instanceModelMatrix * vec4(inPosition, 1.0);
}