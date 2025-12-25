#version 450
#extension GL_KHR_vulkan_glsl : enable
//Nested structures need alignment
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in flat uint fragTextureIndex;
layout(location = 3) in vec3 fragWorldNormal;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textures[64];
layout(set = 0, binding = 2) uniform sampler2D shadowMaps[64];

layout(set = 0, binding = 0) uniform Lights {
    vec4 positions[128]; // or vec4 array for your lights
    vec4 colors[128];
    vec4 ambientColors[128];
    vec4 intensities[128];
    mat4 lightVPs[128];
    uint lightCount;
    uint types[128/4];
} lightsUBO;

void main() {
    vec4 texColor = vec4((texture(textures[fragTextureIndex], fragTexCoord)).xyz * 0.5, 1.0);
    vec3 totalLight = vec3(0.0);
    vec3 totalAmbient = vec3(0.0);

    float shadow = 1.0;

    for (uint i = 0; i < lightsUBO.lightCount; i++) {
    
        shadow = 1.0;
    
        uint idx     = i / 4u; // which uint
        uint bytePos = i % 4u; // which byte

        uint packed = lightsUBO.types[idx];
        uint type   = (packed >> (8u * (3u - bytePos))) & 0xFFu;

        if(type == 0u){ //DIRECTIONAL = 0
            vec3 directionToLight = -lightsUBO.positions[i].xyz;

            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;
            lightColor = lightColor/10.0;

            vec3 diffuseLight = lightColor * max(dot(normalize(fragWorldNormal), normalize(directionToLight)), 0);

            vec4 fragLightSpace = lightsUBO.lightVPs[i] * vec4(fragWorldPos, 1.0);

            vec3 projCoords = fragLightSpace.xyz / fragLightSpace.w;

            if (projCoords.x < -1.0 || projCoords.x > 1.0 ||
                projCoords.y < -1.0 || projCoords.y > 1.0 ||
                projCoords.z <  0.0 || projCoords.z > 1.0)
            {
            
                shadow = 1.0;
            }else{

                projCoords = projCoords * 0.5 + 0.5;

                float closestDepth =
                    texture(shadowMaps[i], projCoords.xy).r;

                float currentDepth = projCoords.z;

                float bias = 0.1;

                if (currentDepth > closestDepth + bias)
                {
                    shadow = currentDepth - (closestDepth + bias);
                }
            
            }

            totalLight += diffuseLight * shadow;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;

        }else if(type == 1u){ //POINT LIGHT = 1

            vec3 directionToLight = lightsUBO.positions[i].xyz - fragWorldPos;
            float distance2 = dot(directionToLight, directionToLight);
            float attenuation = 1.0 / max(distance2, 0.001); // avoid divide by zero

            vec3 lightColor = ((lightsUBO.colors[i].xyz) * lightsUBO.colors[i].w) * lightsUBO.intensities[i].x;

            vec3 diffuseLight = lightColor * max(dot(normalize(fragWorldNormal), normalize(directionToLight)), 0);

            totalLight += diffuseLight * attenuation;
            totalAmbient += lightsUBO.ambientColors[i].xyz * lightsUBO.ambientColors[i].w;
        }
    }

    vec4 lightColor = vec4(totalAmbient + totalLight, 1.0) / lightsUBO.lightCount;
    
    outColor = lightColor + texColor;//vec4(vec3(shadow), 1.0);
}