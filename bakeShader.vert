#version 410 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 texCoord;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 OrthoProj;

out vec2 vertexTexCoord;
out vec4 cameraPos;



void main() {
    cameraPos = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);


    vertexTexCoord = mod(texCoord, 1.0);

    gl_Position = OrthoProj * vec4(mod(texCoord,1.0), 0.0, 1.0);
}