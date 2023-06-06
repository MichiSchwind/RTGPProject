#version 410 core

const float PI = 3.14159265359;

in vec2 interp_UV;
in vec3 N;
in vec3 lightDir;
in vec3 vViewPosition;
in vec4 posLightSpace;

uniform float frequency;
uniform float power;
uniform float timer;
uniform float harmonics;
uniform vec3 colorIn;
// ambient, diffusive and specular components (passed from the application)
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
// weight of the components
// in this case, we can pass separate values from the main application even if Ka+Kd+Ks>1. In more "realistic" situations, I have to set this sum = 1, or at least Kd+Ks = 1, by passing Kd as uniform, and then setting Ks = 1.0-Kd
uniform float Ka;
uniform float Kd;
uniform float Ks;

// shininess coefficients (passed from the application)
uniform float shininess;

// uniforms for GGX model
uniform float alpha; // rugosity - 0 : smooth, 1: rough
uniform float F0; // fresnel reflectance at normal incidence

uniform sampler2D shadowMap;

out vec4 colorFrag;


float Shadow() // this name is the one which is detected by the SetupShaders() function in the main application, and the one used to swap subroutines
{
    // given the fragment position in light coordinates, we apply the perspective divide. Usually, perspective divide is applied in an automatic way to the coordinates saved in the gl_Position variable. In this case, the vertex position in light coordinates has been saved in a separate variable, so we need to do it manually
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    // after the perspective divide the values are in the range [-1,1]: we must convert them in [0,1]
    projCoords = projCoords * 0.5 + 0.5;

    // we get the depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // we calculate an adaptive bias to apply to the currentDepth value, to avoid the shadow acne effect.
    // the bias value is in the range [0.005,0.05]: the final value is calculated considering the angle between the normal and the direction of light
    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    // Version 3: we apply Percentage Close Filtering (PCF) to smooth shadow edges
    float shadow = 0.0;
    // we determine the texel dimension
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    // we sample the depth map considering the 3x3 neighbourhood of the current fragment, and we apply the same test of Version 2 to each sample
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            // we sample the depth map
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            // if the depth (with bias) of the current fragment is greater than the depth in the shadow map, then the fragment is in shadow. We add the result to the shadow variable
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    // we average the shadow result on the kernel size of the PCF
    shadow /= 9.0;

    // To avoid that the areas behind the far plane of the light frustum are considered in shadow, we set their shadow value to 0 (= in light)
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

// We define the different Light Models as functions. This gives us the option to combine them
vec3 LambertianFunc(vec3 diffColor)
{
    vec3 normal = normalize(N);

    vec3 L;
    float lambertian = 0;

    L = normalize(lightDir);

    lambertian += max(dot(L,normal), 0.0);
    

    // Lambert illumination model
    return vec3(Kd * lambertian * diffColor);
}

vec3 PhongFunc(vec3 diffColor)
{
    // ambient component can be calculated at the beginning
    vec3 color = Ka*ambientColor;

    // normalization of the per-fragment normal
    vec3 normal = normalize(N);
    

    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,normal), 0.0);

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( vViewPosition );

        // reflection vector
        vec3 R = reflect(-L, N);

        // cosine of angle between R and V
        float specAngle = max(dot(R, V), 0.0);
        // shininess application to the specular component
        float specular = pow(specAngle, shininess);

        // We add diffusive and specular components to the final color
        // N.B. ): in this implementation, the sum of the components can be different than 1
        color += vec3( Kd * lambertian * diffColor +
                        Ks * specular * specularColor);
    }
    return color;
}

vec3 BlinnPhongFunc(vec3 diffColor)
{
    // ambient component can be calculated at the beginning
    vec3 color = Ka*ambientColor;

    // normalization of the per-fragment normal
    vec3 normal = normalize(N);
    
    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir);

    // Lambert coefficient
    float lambertian = max(dot(L,normal), 0.0);

    // if the lambert coefficient is positive, then I can calculate the specular component
    if(lambertian > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( vViewPosition );

        // Here we use the half vector, not the reflection vector like in the Phong Model
        vec3 H = normalize(L + V);

        // cosine of angle between R and V
        float specAngle = max(dot(H, N), 0.0);
        // shininess application to the specular component
        float specular = pow(specAngle, shininess);

        // We add diffusive and specular components to the final color
        // N.B. ): in this implementation, the sum of the components can be different than 1
        color += vec3( Kd * lambertian * diffColor +
                        Ks * specular * specularColor);
    }
    return color;
}

// Schlick-GGX method for geometry obstruction (used by GGX model)
float G1(float angle, float alpha)
{
    // in case of Image Based Lighting, the k factor is different:
    // usually it is set as k=(alpha*alpha)/2
    float r = (alpha + 1.0);
    float k = (r*r) / 8.0;

    float num   = angle;
    float denom = angle * (1.0 - k) + k;

    return num / denom;
}

vec3 GGXFunc(vec3 diffColor)
{
    // normalization of the per-fragment normal
    vec3 normal = normalize(N);

    // diffusive (Lambert) reflection component
    // I use the value sampled from the texture
    vec3 lambert = (Kd*diffColor)/PI;

    // we initialize the final color
    vec3 color = vec3(0.0);

    // normalization of the per-fragment light incidence direction
    vec3 L = normalize(lightDir);

    // cosine angle between direction of light and normal
    float NdotL = max(dot(normal, L), 0.0);

    // we initialize the specular component
    vec3 specular = vec3(0.0);

    // if the cosine of the angle between direction of light and normal is positive, then I can calculate the specular component
    if(NdotL > 0.0)
    {
        // the view vector has been calculated in the vertex shader, already negated to have direction from the mesh to the camera
        vec3 V = normalize( vViewPosition );

        // half vector
        vec3 H = normalize(L + V);

        // we implement the components seen in the slides for a PBR BRDF
        // we calculate the cosines and parameters to be used in the different components
        float NdotH = max(dot(N, H), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        float alpha_Squared = alpha * alpha;
        float NdotH_Squared = NdotH * NdotH;

        // Geometric factor G2
        // Smith’s method (uses Schlick-GGX method for both geometry obstruction and shadowing )
        float G2 = G1(NdotV, alpha)*G1(NdotL, alpha);

        // Rugosity D
        // GGX Distribution
        float D = alpha_Squared;
        float denom = (NdotH_Squared*(alpha_Squared-1.0)+1.0);
        D /= PI*denom*denom;

        // Fresnel reflectance F (approx Schlick)
        vec3 F = vec3(pow(1.0 - VdotH, 5.0));
        F *= (1.0 - F0);
        F += F0;

        // we put everything together for the specular component
        specular = (F * G2 * D) / (4.0 * NdotV * NdotL);

        // the rendering equation is:
        //integral of: BRDF * Li * (cosine angle between N and L)
        // BRDF in our case is: the sum of Lambert and GGX
        // Li is considered as equal to 1: light is white, and we have not applied attenuation. With colored lights, and with attenuation, the code must be modified and the Li factor must be multiplied to finalColor
        color += (lambert + specular)*NdotL;
    }
    return color;
}


subroutine vec4 fragShaders();

subroutine uniform fragShaders FragmentShader;

subroutine(fragShaders) vec4 LambertianPlusShadow()
{
    float shadow = Shadow();
    vec3 color = LambertianFunc(colorIn);
    return vec4(color, 1.0f);
}

subroutine(fragShaders) vec4 PhongPlusShadw()
{
    float shadow = Shadow();
    vec3 color = PhongFunc(colorIn);
    return vec4((1.0 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 BlinnPhongPlusShadow()
{
    float shadow = Shadow();
    vec3 color = BlinnPhongFunc(colorIn);
    return vec4((1.0 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 GGXPlusShadow()
{
    float shadow = Shadow();
    vec3 color = GGXFunc(colorIn);
    return vec4((1.0 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 normal2ColorPlusLambertian()
{
    return vec4(LambertianFunc(N), 1.0f);
}

subroutine(fragShaders) vec4 normal2ColorPlusBlinnPhong()
{
    
    return vec4(BlinnPhongFunc(N), 1.0f);
}

subroutine(fragShaders) vec4 uv2ColorPlusLambertian()
{
    return vec4(LambertianFunc(vec3(interp_UV,0.0)), 1.0f);
}

subroutine(fragShaders) vec4 uv2ColorPlusBlinnPhong()
{
    return vec4(BlinnPhongFunc(vec3(interp_UV, 0.0f)), 1.0f);
}

subroutine(fragShaders) vec4 FULLCOLOR()
{
    float shadow = Shadow();
    return vec4((1.0 - shadow) * colorIn, 1.0);
}



void main()
{
    colorFrag = FragmentShader();
}