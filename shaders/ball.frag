#version 410 core
in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in float visibility;

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
uniform vec3 fogColor = vec3(0.5, 0.6, 0.7);

// Additional uniforms for enhanced ball lighting
uniform vec3 ballLightPos = vec3(0.0, 5.0, 0.0); 
uniform vec3 ballLightColor = vec3(1.0, 1.0, 1.0);
uniform float ballLightIntensity = 2.0;

float ShadowCalculation(vec4 fragPosLightSpace, float bias) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    return currentDepth - bias > closestDepth ? 0.0 : 1.0;
}

void main() {
    // Basic lighting calculations (from original shader)
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightDirN = normalize(vec3(view * vec4(lightDir, 0.0f)));
    vec3 viewDir = normalize(-fPosEye.xyz);
    
    float bias = max(0.05 * (1.0 - dot(normalEye, lightDirN)), 0.005);
    vec4 fragPosLightSpace = lightSpaceMatrix * model * vec4(fPosition, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, bias);
    
    // Enhanced lighting for the ball
    float ambientStrength = 0.6f;  // Increased ambient
    vec3 ambient = ambientStrength * vec3(1.0f);
    
    // Directional light contribution
    float diff = max(dot(normalEye, lightDirN), 0.0f);
    vec3 diffuse = diff * lightColor;
    
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);  // Reduced shininess
    vec3 specular = spec * lightColor * 1.0f;  // Increased specular intensity
    
    // Additional point light for the ball
    vec3 ballLightPosEye = vec3(view * vec4(ballLightPos, 1.0));
    vec3 ballLightDir = normalize(ballLightPosEye - fPosEye.xyz);
    
    float ballDiff = max(dot(normalEye, ballLightDir), 0.0f);
    vec3 ballDiffuse = ballDiff * ballLightColor * ballLightIntensity;
    
    vec3 ballReflectDir = reflect(-ballLightDir, normalEye);
    float ballSpec = pow(max(dot(viewDir, ballReflectDir), 0.0f), 32.0f);
    vec3 ballSpecular = ballSpec * ballLightColor * ballLightIntensity * 0.5;
    
    // Combine all lighting contributions
    vec4 texColor = texture(diffuseTexture, fTexCoords);
    vec3 result = (ambient + shadow * (diffuse + specular + ballDiffuse + ballSpecular)) * texColor.rgb;
    
    // Apply fog if enabled
    if (density > 0.0) {
        result = mix(fogColor, result, visibility);
    }
    
    fColor = vec4(result, 1.0);
}