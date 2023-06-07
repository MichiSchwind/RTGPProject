#version 410

uniform vec3 colorIn;

out vec4 colorFrag;


void main() {
    colorFrag = vec4(colorIn, 1.0);
}