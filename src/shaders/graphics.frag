#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension  GL_EXT_samplerless_texture_functions : enable


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

layout(set = 1, binding = 3) uniform ShadowLights {
    vec4 positions[64];
    vec4 colors[64];
    vec4 ambientColors[64];
    vec4 intensities[64];
    mat4 lightVPs[64];
    uint lightCount;
    uint types[64/4];
    uint shadowMapIndices[64/4];
} shadowLightsUBO;

layout(set = 0, binding = 4) uniform texture2D shadowMaps[];
layout(set = 0, binding = 5) uniform texture2D textures[];


layout(location = 0) out vec4 outColor;

float shadowCalculation(vec4 fragPosLightSpace, uint shadowIndex)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.z < 0.0 || projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
        return 0.0;

    float shadow = 0.0;

     vec2 texelSize = 1.0 / textureSize(shadowMaps[shadowIndex], 0);

    float closestDepth = 0.0;
    // 3x3 PCF
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            closestDepth = texture(
                sampler2D(shadowMaps[shadowIndex], linearSampler),
                projCoords.xy + vec2(x, y) * texelSize
            ).r;
            
            float bias = 0.001;

            shadow += projCoords.z + bias > closestDepth  ? 0.9 : 0.0;
        }
    }

    shadow /= 9.0; // average
    return shadow;
}

void main(){
    vec4 texColor = texture(sampler2D(textures[texIndex], linearSampler), texCoord);
    vec3 totalLight = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);

    float shadow = 0;

    for (uint i = 0; i < lightsUBO.lightCount; i++) {
    
        uint idx = i / 4u; // which uint
        uint bytePos = i % 4u; // which byte

        uint packed = lightsUBO.types[idx];
        uint type   = (packed >> (8u * (3u - bytePos))) & 0xFFu;

        if (type == 1u){
            vec3 directionToLight = normalize(lightsUBO.positions[i].xyz - worldPos);
            float distance2 = dot(directionToLight, directionToLight);
            float attenuation = 1.0 / max(distance2, 0.001); // avoid divide by zero

            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;

            vec3 diffuseLight = lightColor * max(dot(normalize(worldNormal), normalize(directionToLight)), 0);

            totalLight += diffuseLight * attenuation;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;
        }
    }

    for (uint i = 0; i < shadowLightsUBO.lightCount; i++) {
    
        uint idx = i / 4u; // which uint
        uint bytePos = i % 4u; // which byte

        uint packed = shadowLightsUBO.types[idx];
        uint type   = (packed >> (8u * (3u - bytePos))) & 0xFFu;

        uint indexPacked = shadowLightsUBO.shadowMapIndices[idx];
        uint shadowIndex = (indexPacked >> (8u * (3u - bytePos))) & 0xFFu;

        if(type == 0u){ //DIRECTIONAL
            vec3 directionToLight = normalize(shadowLightsUBO.positions[i].xyz);
            vec3 lightColor = ((shadowLightsUBO.colors[i].xyz) * shadowLightsUBO.colors[i].w) * shadowLightsUBO.intensities[i].x;
            lightColor = lightColor/2.0;

            vec4 fragLightPos = shadowLightsUBO.lightVPs[i] * vec4(worldPos,1.0);
            shadow = shadowCalculation(fragLightPos, shadowIndex);
            
            vec3 diffuseLight = lightColor * max(dot(normalize(worldNormal), normalize(-directionToLight)), 0) * (1.0 - shadow);

            totalLight += diffuseLight;
            totalAmbient += shadowLightsUBO.ambientColors[i].xyz * shadowLightsUBO.ambientColors[i].w;

        }
    }
    
    vec3 lightColor = totalAmbient + totalLight; // / (lightsUBO.lightCount + shadowLightsUBO.lightCount);
    vec3 finalColor = texColor.rgb * 0.4 + lightColor * 0.6;
	outColor = vec4(finalColor, 1.0);
}