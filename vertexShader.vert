#version 410 core

layout (location = 0) in vec3 position;

layout (location = 1) in vec3 normal;

layout (location = 2) in vec2 UV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

out vec2 interp_UV;
out vec3 N;

void main() 
{
    interp_UV = UV;
    N = normalMatrix * normal; 

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}