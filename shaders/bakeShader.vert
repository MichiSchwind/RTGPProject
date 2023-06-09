#version 410 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 UV;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 OrthoProj;

out vec2 interp_UV;
out vec4 sPos;



void main() {
    // we need the coordinates of the vertex in screen space, so we can compare it with values in the texture 
    sPos = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

    // interpolated UV coordinates
    interp_UV = mod(UV, 1.0);

    // draw at the UV coordinates. This links the paint texture to the mesh
    gl_Position = OrthoProj * vec4(mod(UV,1.0), 0.0, 1.0);
}