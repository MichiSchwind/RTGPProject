#version 410 core

const float PI = 3.14159265359;


////////////////////////////////////////////// INFORMATION FROM THE VERTEX SHADER ////////////////////////////////////////////////////////////////////
// Position of the Vertex and the Light in various coordinate Spaces
in vec3 vViewPosition;
in vec4 posInWorldCoords;
in vec4 screenPos;

in vec3 lPos;
in vec4 lPosScreen;
in vec3 lightDir;

// UV coordinates of the Vertex
in vec2 interp_UV;

// Normal of the Vertex
in vec3 N;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////// DEKLARATION OF UNIFORMS //////////////////////////////////////////////////////////////////////////
// If the Model to be rendered is supposed to be in one Color then this color gets send to colorIN
uniform vec3 colorIn;

// Information for the light Models (Lambertian, Phong, BlinnPhong and GGX)
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;

uniform float Ka;
uniform float Kd;
uniform float Ks;

uniform float shininess;

uniform float alpha;
uniform float F0; 

// repetition of the UV coordinates for the paint texture
uniform float uvRep;

// repetition of the UV coordinates for the enviroment Models
uniform float texRep;

// how far is the far plane in the shadowMap Cubetexture
uniform float far_plane;

// uniforms for different Textures
// shadowMap Cubetexture
uniform samplerCube shadowMap;
// the texture for the enviroment Models
uniform sampler2D textureID;
// the paint texture. This is where you draw in
uniform sampler2D bakeTexture;

uniform float frequency;
uniform float power;
uniform float timer;
uniform float harmonics;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

out vec4 colorFrag;

//////////////////////////////////////////////////////// DEKLARATION OF FUNCTIONS ////////////////////////////////////////////////////////////////////
// get the Color of the texture Mesh
vec3 getMeshColor();

// we calculate the Brightness given the distance to the lightsource
float calculateBrightness(float distance, float attenuation);

// lookup in the shadowcubemap if the vertex is in shadow or not
float Shadow(); 

// calculate the brightness with the different light Models
vec3 LambertianFunc(vec3 diffColor);

vec3 PhongFunc(vec3 diffColor);

vec3 BlinnPhongFunc(vec3 diffColor);

float G1(float angle, float alpha);

vec3 GGXFunc(vec3 diffColor);

// functions for random/regular patterns
vec3 mod289(vec3 x);

vec4 mod289(vec4 x);

vec4 permute(vec4 x);

vec4 taylorInvSqrt(vec4 r);

float snoise(vec3 v);

float aastep(float threshold, float value);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////// SUBROUTINES ////////////////////////////////////////////////////////////////////////
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
    return vec4((1.1 - shadow) * color, 1.0f);
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
    return vec4((1.1 - shadow) * color, 1.0f);
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
    return vec4((1.1 - shadow) * color, 1.0f);
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
    return vec4((1.1 - shadow) * color, 1.0f);
}

subroutine(fragShaders) vec4 NCAPlusBlinnPhong()
{
    float r = power*snoise(vec3(interp_UV*frequency, 0.4*timer));
    float g = power*snoise(vec3(interp_UV*frequency, -0.7*timer));
    float b = power*snoise(vec3(interp_UV*frequency, 0.8*timer));

    vec3 color = vec3(r,g,b);
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = BlinnPhongFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
}

subroutine(fragShaders)  vec4 TAPlusBlinnPhong()
{
    float p = power;
    float f = frequency;

    float value = 0.0;
    for (int i=0;i<harmonics;i++)
    {
        value += p*abs(snoise(vec3(interp_UV*f, 0.0)));
        p*=0.5;
        f*=2.0;
    }

    vec3 color = vec3(value);
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = BlinnPhongFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
}

subroutine(fragShaders) vec4 StripesSmoothstepPlusBlinnPhong() 
{
    float k = fract(interp_UV.s * 5.0);
    k = abs(2.0*k -1.0);

    float f = smoothstep(0.45,0.5,k);

    vec3 color = mix(colorIn, vec3(0.0,0.0,0.0), f);
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = BlinnPhongFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
}

subroutine(fragShaders) vec4 CirclesSmoothstepPlusBlinnPhong() 
{
    vec2 k = fract(interp_UV * 5.0);
    float f = smoothstep(0.3, 0.32, length(k-0.5));
    
    vec3 color = mix(colorIn, vec3(0.0,0.0,0.0), f);
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = BlinnPhongFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
}

subroutine(fragShaders) vec4 FULLCOLOR()
{
    float shadow = Shadow();
    vec3 color = calculateBrightness(length(posInWorldCoords.xyz - lPos), 0.3f) * colorIn;
    return vec4((1.1 - shadow) * color, 1.0);
}

subroutine(fragShaders) vec4 Bloom()
{
    vec3 color = vec3(1.0,1.0,1.0);
    float dist = length(lPos - (posInWorldCoords.xyz / posInWorldCoords.w));

    color = (10000.0 / pow(1 + dist, 80.0)) * color;

    return vec4(color, 1.0);
}

subroutine(fragShaders) vec4 Texture()
{
    float shadow = Shadow();
    vec3 color = texture(textureID, mod(texRep * interp_UV,1.0)).rgb;
    color = calculateBrightness(length(posInWorldCoords.xyz - lPos), 0.3f) * color;
    return vec4((1.1 - shadow) * color, 1.0);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////// MAIN ////////////////////////////////////////////////////////////////////////////
void main()
{
    colorFrag = FragmentShader();
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////// DEFINITION OF FUNCTIONS //////////////////////////////////////////////////////////////////////////
vec3 getMeshColor()
{
    vec2 repUV = mod(uvRep * interp_UV, 1.0);

    vec3 color = texture(bakeTexture, repUV).rgb;

    return color;
}


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
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}  

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

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }

float aastep(float threshold, float value) 
{
  float afwidth = 0.7 * length(vec2(dFdx(value), dFdy(value)));

  return smoothstep(threshold-afwidth, threshold+afwidth, value);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////