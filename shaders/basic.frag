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


uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform float spotLightCutOff;
uniform float spotLightOuterCutOff;


uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;


uniform sampler2D metallicTexture;
uniform sampler2D roughnessTexture;
uniform bool usePBR = false;
uniform mat4 lightSpaceMatrix;
uniform float density;
uniform bool isWhite = false;
in float visibility;
uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);
uniform bool smoothShading = false;

float ShadowCalculation(vec4 fragPosLightSpace, float bias) {

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    return currentDepth - bias > closestDepth ? 0.0 : 1.0;
}

vec3 calculateSpotLight(vec3 normal, vec3 viewDir, vec3 fragPos) {
    
    vec3 spotLightPosView = vec3(view * vec4(spotLightPosition, 1.0));
    vec3 spotLightDirView = normalize(vec3(view * vec4(spotLightDirection, 0.0)));
    vec3 lightToFrag = normalize(spotLightPosView - fragPos);
    
    float theta = dot(lightToFrag, -spotLightDirView);
    float epsilon = spotLightCutOff - spotLightOuterCutOff;
    float intensity = clamp((theta - spotLightOuterCutOff) / epsilon, 0.0, 1.0);
    
    if(theta > spotLightOuterCutOff) {

        float distance = length(spotLightPosView - fragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        float metallic = usePBR ? texture(metallicTexture, fTexCoords).r : 1.0;
        float roughness = usePBR ? texture(roughnessTexture, fTexCoords).r : 0.5;
        
        float diff = max(dot(normal, lightToFrag), 0.0);
        vec3 diffuse = diff * spotLightColor;
        
        vec3 reflectDir = reflect(-lightToFrag, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), mix(32.0, 1.0, roughness));
        vec3 specular = spec * spotLightColor * metallic;
        
        return (diffuse + specular) * attenuation * intensity;
    }
    return vec3(0.0);
}

void main() {

    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightDirN = normalize(vec3(view * vec4(lightDir, 0.0f)));
    vec3 viewDir = normalize(-fPosEye.xyz);
    
    float bias = max(0.05 * (1.0 - dot(normalEye, lightDirN)), 0.005);
    vec4 fragPosLightSpace = lightSpaceMatrix * model * vec4(fPosition, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, bias);
    
    float metallic = usePBR ? texture(metallicTexture, fTexCoords).r : 1.0;
    float roughness = usePBR ? texture(roughnessTexture, fTexCoords).r : 0.5;
    
    float ambientStrength = 0.5f;
    vec3 ambient = ambientStrength * vec3(1.0f);
    float diff = max(dot(normalEye, lightDirN), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), mix(64.0f, 1.0f, roughness));
    vec3 specular = spec * lightColor * (usePBR ? metallic : 0.7f);
    
    vec3 spotLightContrib = calculateSpotLight(normalEye, viewDir, fPosEye.xyz);
    
    vec3 result;
    if (isWhite) {
        result = (ambient + shadow * (diffuse + specular + spotLightContrib)) * vec3(1.0);
    } else {
        vec4 texColor = texture(diffuseTexture, fTexCoords);
        result = (ambient + shadow * (diffuse + specular + spotLightContrib)) * texColor.rgb;
    }
    
    if (density > 0.0) {
        result = mix(fogColor, result, visibility);
    }
   

    if (smoothShading) {
        float edgeSmoothness = 0.5;
        float edgeFactor = dot(normalEye, viewDir);
        edgeFactor = smoothstep(0.0, edgeSmoothness, edgeFactor);
        result *= edgeFactor;
    }

    fColor = vec4(result, 1.0);
}