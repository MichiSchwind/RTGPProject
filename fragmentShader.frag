#version 410 core

const float PI = 3.14159265359;

in vec2 interp_UV;
in vec3 N;
in vec3 lightDir;
in vec3 vViewPosition;

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

out vec4 colorFrag;

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

subroutine(fragShaders) vec4 Lambertian()
{
    return vec4(LambertianFunc(colorIn), 1.0f);
}

subroutine(fragShaders) vec4 Phong()
{
    return vec4(PhongFunc(colorIn), 1.0f);
}

subroutine(fragShaders) vec4 BlinnPhong()
{
    return vec4(BlinnPhongFunc(colorIn), 1.0f);
}

subroutine(fragShaders) vec4 GGX()
{
    return vec4(GGXFunc(colorIn), 1.0f);
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
    return vec4(colorIn, 1.0);
}



void main()
{
    colorFrag = FragmentShader();
}