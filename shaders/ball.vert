#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in float visibility;

out vec4 fColor;

uniform mat4 view;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D diffuseTexture;
uniform sampler2D shadowMap;
uniform float density;
uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);

float ShadowCalculation(vec4 fragPosLightSpace, float bias) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0)
        return 0.0;
        
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    return shadow;
}

void main() {

    vec3 normal = normalize(fNormal);
    vec3 lightDirNorm = normalize(-lightDir);
    

    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;
    

    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;
    
    
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fPosition);
    vec3 reflectDir = reflect(-lightDirNorm, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    
    
    float bias = max(0.05 * (1.0 - dot(normal, lightDirNorm)), 0.005);
    float shadow = ShadowCalculation(fragPosLightSpace, bias);
    
    
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * texture(diffuseTexture, fTexCoords).rgb;
    
    
    if(density > 0.0) {
        result = mix(fogColor, result, visibility);
    }
    
    fColor = vec4(result, 1.0);
}