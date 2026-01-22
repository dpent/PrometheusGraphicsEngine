#version 450
#extension GL_EXT_nonuniform_qualifier : enable
#extension  GL_EXT_samplerless_texture_functions : enable


layout(location = 0) in vec3 vertColor;
layout(location = 1) in flat uint objectIndex;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in flat uint texIndex;
layout(location = 4) in vec3 worldPos;
layout(location = 5) in vec3 worldNormal;
layout(location = 6) in flat uint useNormalMap;

layout(set = 0, binding = 0) uniform sampler linearSampler;
layout(set = 0, binding = 2) uniform Lights {
    vec4 positions[128];
    vec4 colors[128];
    vec4 ambientColors[128];
    vec4 intensities[128];
    mat4 lightVPs[128];
    uint lightCount;
    uvec4 types[128/16];
} lightsUBO;

layout(set = 1, binding = 3) uniform ShadowLights {
    vec4 positions[64];
    vec4 colors[64];
    vec4 ambientColors[64];
    vec4 intensities[64];
    mat4 lightVPs[64];
    uint lightCount;
    uvec4 types[64/16];
    uvec4 shadowMapIndices[64/16];
} shadowLightsUBO;

layout(set = 1, binding = 4) uniform texture2D shadowMaps[];
layout(set = 0, binding = 5) uniform texture2D textures[];


layout(location = 0) out vec4 outColor;

float rand(float co) { return fract(sin(co*(91.3458)) * 47453.5453); }
float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }

float pcss(uint shadowIndex, vec3 projCoords, float lightSize)
{
    float zReceiver = projCoords.z;

    vec2 uv = projCoords.xy;
    float texelSize = 1.0 / float(textureSize(shadowMaps[shadowIndex], 0).x);
    
    const int blockerSearchSamples = 16;
    float avgBlockerDepth = 0.0;
    int blockers = 0;

    float bias = 0.0001;

    for(int j = 0; j < 5; ++j)
    {
        for(int i = 0; i < blockerSearchSamples; ++i)
        {
            float angle = float(i) * 6.2831853 / float(blockerSearchSamples);
            vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * (1.0f + j);

            float depth = texture(sampler2D(shadowMaps[shadowIndex], linearSampler), uv + offset).r;

            if(depth < zReceiver)
            {
                avgBlockerDepth += depth;
                blockers++;
            }
        }
    }

    if(blockers == 0)
        return 0.0;

    avgBlockerDepth /= float(blockers);

    float penumbra = ((zReceiver - avgBlockerDepth) * (lightSize)) / avgBlockerDepth; 
    const int pcfSamples = 128; 
    float shadow = 0.0;

    for(int i = 0; i < pcfSamples; ++i) 
    { 
        vec2 randOffset = vec2(rand(uv.x + i), rand(uv.y + i));
        float angle = float(i) * 6.2831853 / float(pcfSamples); 
        vec2 offset = vec2(cos(angle), sin(angle)) * texelSize * penumbra;

        float depth = texture(sampler2D(shadowMaps[shadowIndex], linearSampler), uv + offset).r; 

        bool inShadow = (zReceiver + bias > depth);
        if (inShadow) {
            shadow += 1.0;
        }
    }
    shadow/=float(pcfSamples);

    return shadow;
}

float shadowCalculation(vec4 fragPosLightSpace, uint shadowIndex, float lightSize)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords.xy = projCoords.xy * 0.5 + 0.5;

    if (projCoords.z > 1.0 || projCoords.z < 0.0 || projCoords.x > 1.0 || projCoords.x < 0.0 || projCoords.y > 1.0 || projCoords.y < 0.0)
        return 0.0;

    float shadow = 0.0;

     shadow = pcss(shadowIndex, projCoords, lightSize);

    return shadow;
}

void main(){
    vec4 texColor = texture(sampler2D(textures[texIndex], linearSampler), texCoord);
    vec3 totalLight = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);

    float shadow = 0;

    for (uint i = 0; i < lightsUBO.lightCount; i++) {

        uint uvecIndex  = i / 16u;       // which uvec4
        uint elementIdx = (i / 4u) % 4u; // which uint inside the uvec4
        uint bytePos    = i % 4u;        // which byte inside the uint

        uint packedType = lightsUBO.types[uvecIndex][elementIdx];
        uint type = (packedType >> (8u * (3u - bytePos))) & 0xFFu;

        if (type == 1u){
            vec3 directionToLight = normalize(lightsUBO.positions[i].xyz - worldPos);
            float distance2 = dot(directionToLight, directionToLight);
            float attenuation = 1.0 / max(distance2, 0.001); // avoid divide by zero

            vec3 lightColor = lightsUBO.colors[i].xyz * lightsUBO.intensities[i].x;

            vec3 diffuseLight;

            if(useNormalMap != 0u){
                diffuseLight = lightColor * max(dot(texture(sampler2D(textures[texIndex + 1], linearSampler), texCoord).xyz, normalize(directionToLight)), 0);
            }else{
                diffuseLight = lightColor * max(dot(normalize(worldNormal), normalize(-directionToLight)), 0) * (1.0 - shadow);
            }

            totalLight += diffuseLight * attenuation;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;
        }
    }

    for (uint i = 0; i < shadowLightsUBO.lightCount; i++) {
    
        uint uvecIndex  = i / 16u;       // which uvec4
        uint elementIdx = (i / 4u) % 4u; // which uint inside the uvec4
        uint bytePos    = i % 4u;        // which byte inside the uint

        uint packedType = shadowLightsUBO.types[uvecIndex][elementIdx];
        uint type = (packedType >> (8u * (3u - bytePos))) & 0xFFu;

        uint packedIndex = shadowLightsUBO.shadowMapIndices[uvecIndex][elementIdx];
        uint shadowIndex = (packedIndex >> (8u * (3u - bytePos))) & 0xFFu;

        if(type == 0u){ //DIRECTIONAL
            vec3 directionToLight = normalize(shadowLightsUBO.positions[i].xyz);
            vec3 lightColor = shadowLightsUBO.colors[i].xyz * shadowLightsUBO.intensities[i].x;
            lightColor = lightColor/2.0;

            vec4 fragLightPos = shadowLightsUBO.lightVPs[i] * vec4(worldPos,1.0);
            shadow = shadowCalculation(fragLightPos, shadowIndex, shadowLightsUBO.colors[i].w);

            vec3 diffuseLight;

            if(useNormalMap != 0u){
                diffuseLight = lightColor * max(dot(texture(sampler2D(textures[texIndex + 1], linearSampler), texCoord).xyz, normalize(-directionToLight)), 0) * (1.0 - shadow);
            }else{
                diffuseLight = lightColor * max(dot(normalize(worldNormal), normalize(-directionToLight)), 0) * (1.0 - shadow);
            }
            
            totalLight += diffuseLight;
            totalAmbient += shadowLightsUBO.ambientColors[i].xyz * shadowLightsUBO.ambientColors[i].w;

        }
    }

    vec3 lightColor = totalAmbient + totalLight;
    vec3 finalColor = texColor.rgb * lightColor;
	outColor = vec4(finalColor, 1.0);
}