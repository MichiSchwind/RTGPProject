// simple vertex Shader - Take Mouse position and draw it on the screen
#version 410

layout (location = 0) in vec2 position;

void main() 
{
    gl_Position = vec4(position, 0.0, 1.0);
}