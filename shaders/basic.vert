#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out float visibility;
uniform float density = 0.007;
uniform float gradient = 1.5;

void main() {
    vec4 positionRelativeToCam = view * model * vec4(vPosition, 1.0);
    float distance = length(positionRelativeToCam.xyz);
    visibility = exp(-pow((distance * density), gradient));
    visibility = clamp(visibility, 0.0, 1.0);
    
    gl_Position = projection * positionRelativeToCam;
    fPosition = vPosition;
    fNormal = vNormal;
    fTexCoords = vTexCoords;
}