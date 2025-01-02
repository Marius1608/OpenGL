#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
out vec4 fColor;

uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform float density;
in float visibility;
uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);

float ShadowCalculation(vec4 fragPosLightSpace, float bias) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get closest depth value from light's perspective
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get current depth value
    float currentDepth = projCoords.z;
    
    // Check whether current frag pos is in shadow
    return currentDepth - bias > closestDepth ? 0.0 : 1.0;
}

void main() {

    // Lighting calculations in eye space
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightDirN = normalize(vec3(view * vec4(lightDir, 0.0f)));
    vec3 viewDir = normalize(-fPosEye.xyz);
    
    // Calculate bias for shadow mapping
    float bias = max(0.05 * (1.0 - dot(normalEye, lightDirN)), 0.005);
    
    // Calculate shadow
    vec4 fragPosLightSpace = lightSpaceMatrix * model * vec4(fPosition, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, bias);
    
    // Lighting components
    float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * vec3(1.0f);
    
    float diff = max(dot(normalEye, lightDirN), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 64.0f);
    vec3 specular = spec * lightColor * 0.7f;
    
    vec4 texColor = texture(diffuseTexture, fTexCoords);
    vec3 result = (ambient + shadow * (diffuse + specular)) * texColor.rgb;
    
    if (density > 0.0) {
        result = mix(fogColor, result, visibility);
    }
    
    fColor = vec4(result, 1.0);
}