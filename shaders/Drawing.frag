// simple fragment Shader - let user decide in which color he wants to paint 
#version 410

uniform float[3] colorIn;

out vec4 colorFrag;


void main() {
    colorFrag = vec4(colorIn[0], colorIn[1], colorIn[2], 1.0);
}