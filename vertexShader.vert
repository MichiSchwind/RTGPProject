#version 410 core


layout (location = 0) in vec3 position;

layout (location = 1) in vec3 normal;

layout (location = 2) in vec2 UV;


uniform vec3 lightPos;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

out vec2 interp_UV;
out vec3 N;
out vec3 lightDir;
out vec3 vViewPosition;
out vec4 posLightSpace;

void main() 
{
    interp_UV = UV;
    N = normalize(normalMatrix * normal); 

    vec4 posInViewCoords = viewMatrix * modelMatrix * vec4(position, 1.0f);


    vec4 lightsInViewCoords = viewMatrix * vec4(lightPos,1.0);
    lightDir = lightsInViewCoords.xyz -  posInViewCoords.xyz;
    
    vViewPosition = -posInViewCoords.xyz;

    gl_Position = projectionMatrix * posInViewCoords;

    posLightSpace = lightSpaceMatrix * modelMatrix * vec4(position, 1.0f);
}