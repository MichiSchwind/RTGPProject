#version 410 core

const float PI = 3.14159265359;

in vec2 interp_UV;
in vec3 lPos;
in vec3 lightDir;
in vec3 N;
in vec3 vViewPosition;
in vec4 screenPos;
in vec4 posInWorldCoords;

uniform float frequency;
uniform float power;
uniform float harmonics;
uniform vec3 colorIn;



uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

uniform float Ka;
uniform float Kd;
uniform float Ks;


uniform float shininess;


uniform float alpha;
uniform float F0; 

uniform float uvRep;
uniform float far_plane;
uniform samplerCube shadowMap;
uniform sampler2D bakeTexture;

out vec4 colorFrag;


vec3 getMeshColor()
{
    vec2 repUV = mod(uvRep * interp_UV, 1.0);

    vec3 color = texture(bakeTexture, repUV).rgb;

    return color;
}

/*float Shadow()
{
    
    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
   
    projCoords = projCoords * 0.5 + 0.5;

   
    float currentDepth = projCoords.z;

    vec3 normal = normalize(N);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);


    float shadow = 0.0;
   
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {

            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;

            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }

    shadow /= 9.0;


    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}*/
float calculateBrightness(float distance, float attenuation)
{
    const float constant = 1.0f;
    const float linear = 0.09f;
    const float quadratic = 0.032f;
    
    float intensity = 1.0f / (constant + linear * distance + quadratic * (distance * distance));
    float decayedBrightness = intensity / attenuation;
    
    return decayedBrightness;
}

float Shadow()
{
    // get vector between fragment position and light position
    vec3 fragToLight = posInWorldCoords.xyz - lPos;

    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(shadowMap, fragToLight).r;

    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= far_plane;

    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);

    // now test for shadows
    float bias = 0.5; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

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
    vec3 color = Ka*ambientColor;

    vec3 normal = normalize(N);
    

    vec3 L = normalize(lightDir);


    float lambertian = max(dot(L,normal), 0.0);

    if(lambertian > 0.0)
    {

        vec3 V = normalize( vViewPosition );

        vec3 R = reflect(-L, N);

       
        float specAngle = max(dot(R, V), 0.0);

        float specular = pow(specAngle, shininess);

     
        color += vec3( Kd * lambertian * diffColor +
                        Ks * specular * specularColor);
    }
    return color;
}

vec3 BlinnPhongFunc(vec3 diffColor)
{
    vec3 color = Ka*ambientColor;


    vec3 normal = normalize(N);
    
   
    vec3 L = normalize(lightDir);


    float lambertian = max(dot(L,normal), 0.0);

    if(lambertian > 0.0)
    {
        vec3 V = normalize( vViewPosition );

        vec3 H = normalize(L + V);


        float specAngle = max(dot(H, N), 0.0);

        float specular = pow(specAngle, shininess);

        color += vec3( Kd * lambertian * diffColor +
                        Ks * specular * specularColor);
    }
    return color;
}


float G1(float angle, float alpha)
{

    float r = (alpha + 1.0);
    float k = (r*r) / 8.0;

    float num   = angle;
    float denom = angle * (1.0 - k) + k;

    return num / denom;
}

vec3 GGXFunc(vec3 diffColor)
{
    
    vec3 normal = normalize(N);


    vec3 lambert = (Kd*diffColor)/PI;


    vec3 color = vec3(0.0);


    vec3 L = normalize(lightDir);


    float NdotL = max(dot(normal, L), 0.0);


    vec3 specular = vec3(0.0);


    if(NdotL > 0.0)
    {

        vec3 V = normalize( vViewPosition );

 
        vec3 H = normalize(L + V);


        float NdotH = max(dot(N, H), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float VdotH = max(dot(V, H), 0.0);
        float alpha_Squared = alpha * alpha;
        float NdotH_Squared = NdotH * NdotH;


        float G2 = G1(NdotV, alpha)*G1(NdotL, alpha);

        float D = alpha_Squared;
        float denom = (NdotH_Squared*(alpha_Squared-1.0)+1.0);
        D /= PI*denom*denom;

  
        vec3 F = vec3(pow(1.0 - VdotH, 5.0));
        F *= (1.0 - F0);
        F += F0;


        specular = (F * G2 * D) / (4.0 * NdotV * NdotL);


        color += (lambert + specular)*NdotL;
    }
    return color;
}


subroutine vec4 fragShaders();

subroutine uniform fragShaders FragmentShader;

subroutine(fragShaders) vec4 LambertianPlusShadow()
{
    float shadow = Shadow();
    vec3 color = colorIn;
    vec3 paint = getMeshColor();
    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = LambertianFunc(color);
    return vec4((0.9 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 PhongPlusShadw()
{
    float shadow = Shadow();
    vec3 color = colorIn;
    vec3 paint = getMeshColor();
    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }
    color = PhongFunc(color);
    return vec4((0.9 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 BlinnPhongPlusShadow()
{
    float shadow = Shadow();
    vec3 color = colorIn;
    vec3 paint = getMeshColor();
    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }
    color = BlinnPhongFunc(color);
    return vec4((0.9 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 GGXPlusShadow()
{
    float shadow = Shadow();
    vec3 color = colorIn;
    vec3 paint = getMeshColor();
    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }
    color = GGXFunc(color);
    return vec4((0.9 - shadow) * color, 1.0f);
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
    vec3 color = calculateBrightness(length(posInWorldCoords.xyz - lPos), 0.3f) * colorIn;
    return vec4((0.9 - shadow) * color, 1.0);
}



void main()
{
    colorFrag = FragmentShader();
}