#version 410 core

out vec4 colorFrag;

in vec2 interp_UV;
in vec3 N;

uniform float frequency;
uniform float power;
uniform float timer;
uniform float harmonics;
uniform vec3 colorIn;


subroutine vec4 fragShaders();

subroutine uniform fragShaders FragmentShader;

subroutine(fragShaders) vec4 Blue()
{
    return vec4(0.0f,0.0f,1.0f, 1.0f);
}

subroutine(fragShaders) vec4 Red()
{
    return vec4(1.0f,0.0f,0.0f, 1.0f);
}

subroutine(fragShaders) vec4 Yellow()
{
    return vec4(1.0f,1.0f,0.0f, 1.0f);
}

subroutine(fragShaders) vec4 Green()
{
    return vec4(0.0f,1.0f,0.0f, 1.0f);
}

subroutine(fragShaders) vec4 onecolor()
{
    return vec4(colorIn, 1.0f);
}

subroutine(fragShaders) vec4 randomNoise()
{
    //Insert random Noise Algorithm
    return vec4(0.0f, 1.0f, 0.0f , 1.0f);
}

subroutine(fragShaders) vec4 normal2color()
{
    return vec4(N, 1.0f);
}

subroutine(fragShaders) vec4 uv2color()
{
    return vec4(interp_UV, 0.0f, 1.0f);
}



void main()
{
    colorFrag = FragmentShader();
}