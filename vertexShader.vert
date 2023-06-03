#version 410 core

#define numLights 4

layout (location = 0) in vec3 position;

layout (location = 1) in vec3 normal;

layout (location = 2) in vec2 UV;


uniform vec3 lights[numLights];
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out vec2 interp_UV;
out vec3 N;
out vec3 lightDir[numLights];
out vec3 vViewPosition;

void main() 
{
    interp_UV = UV;
    N = normalize(normalMatrix * normal); 

    vec4 posInViewCoords = viewMatrix * modelMatrix * vec4(position, 1.0f);

    for (int i= 0; i < numLights; i++){
        vec4 lightsInViewCoords = viewMatrix * vec4(lights[i],1.0);
        lightDir[i] = lightsInViewCoords.xyz -  posInViewCoords.xyz;
    }
    
    vViewPosition = -posInViewCoords.xyz;

    gl_Position = projectionMatrix * posInViewCoords;
}