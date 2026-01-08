#version 450
#extension GL_EXT_nonuniform_qualifier : enable


layout(location = 0) in vec3 vertColor;
layout(location = 1) in flat uint objectIndex;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in flat uint texIndex;
layout(location = 4) in vec3 worldPos;
layout(location = 5) in vec3 worldNormal;

layout(set = 0, binding = 0) uniform sampler linearSampler;
layout(set = 0, binding = 2) uniform Lights {
    vec4 positions[128];
    vec4 colors[128];
    vec4 ambientColors[128];
    vec4 intensities[128];
    mat4 lightVPs[128];
    uint lightCount;
    uint types[128/4];
} lightsUBO;
layout(set = 0, binding = 3) uniform texture2D textures[];


layout(location = 0) out vec4 outColor;

void main(){
    vec4 texColor = texture(sampler2D(textures[texIndex], linearSampler), texCoord);
    vec3 totalLight = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);

    for (uint i = 0; i < lightsUBO.lightCount; i++) {
    
        uint idx = i / 4u; // which uint
        uint bytePos = i % 4u; // which byte

        uint packed = lightsUBO.types[idx];
        uint type   = (packed >> (8u * (3u - bytePos))) & 0xFFu;

        if(type == 0u){ //DIRECTIONAL
            vec3 directionToLight = lightsUBO.positions[i].xyz;
            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;
            lightColor = lightColor/2.0;

            vec3 diffuseLight = lightColor * max(dot(normalize(worldNormal), normalize(-directionToLight)), 0);

            totalLight += diffuseLight;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;

        }else if (type == 1u){
            vec3 directionToLight = lightsUBO.positions[i].xyz - worldPos;
            float distance2 = dot(directionToLight, directionToLight);
            float attenuation = 1.0 / max(distance2, 0.001); // avoid divide by zero

            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;

            vec3 diffuseLight = lightColor * max(dot(normalize(worldNormal), normalize(directionToLight)), 0);

            totalLight += diffuseLight * attenuation;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;
        }
    }
    
    vec3 lightColor = vec3(totalAmbient + totalLight) / lightsUBO.lightCount;
	outColor = texColor + vec4(lightColor, 1.0);
}