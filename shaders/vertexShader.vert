#version 410 core


layout (location = 0) in vec3 position;

layout (location = 1) in vec3 normal;

layout (location = 2) in vec2 UV;

uniform vec3 lightPos;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out vec3 lPos;
out vec4 lPosScreen;
out vec3 lightDir;
out vec2 interp_UV;
out vec3 N;
out vec3 vViewPosition;
out vec4 screenPos;
out vec4 posInWorldCoords;

// set up a bunch of information for the different fragment shader subroutines 
void main() 
{
    interp_UV = UV;
    N = normalize(normalMatrix * normal); 

    posInWorldCoords = modelMatrix * vec4(position, 1.0f);

    vec4 posInViewCoords = viewMatrix * modelMatrix * vec4(position, 1.0f);

    lPos = lightPos;

    lPosScreen = projectionMatrix * viewMatrix * vec4(lPos,1.0);

    lightDir = (viewMatrix * (vec4(lightPos,1.0) - posInWorldCoords)).xyz;
    
    vViewPosition = -posInViewCoords.xyz;

    screenPos = projectionMatrix * posInViewCoords;

    gl_Position = projectionMatrix * posInViewCoords;

}