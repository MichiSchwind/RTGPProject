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
vec2 hash(vec2 p);

float voronoiNoise(vec3 position);

vec3 voronoiDiagram(vec3 position);

vec3 setColors(vec2 cell);

vec3 palette( float t ); 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////// SUBROUTINES ////////////////////////////////////////////////////////////////////////
subroutine vec4 fragShaders();

subroutine uniform fragShaders FragmentShader;

subroutine(fragShaders) vec4 LambertianPlusShadow()
{
    float shadow = Shadow();
    vec3 color = vec3(0.0,0.0,0.0);
    vec3 paint = getMeshColor();
    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    if (color.x + color.y + color.z == 0.0)
    {
        discard;
    }

    color = LambertianFunc(color);
    return vec4(color, 1.0f);
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

subroutine(fragShaders)  vec4 AnimatedCellsPlusGGX()
{
    float p = power;
    float f = frequency;

    float value = 0.0;
    for (int i=0;i<harmonics;i++)
    {
        value += p*voronoiNoise(vec3(interp_UV*f, timer));
        p*=0.5;
        f*=2.0;
    }

    vec3 color = vec3(value);
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = GGXFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
}

subroutine(fragShaders) vec4 AnimatedColorsPlusGGX()
{
    //float r = power*voronoiNoise(vec3(interp_UV*frequency, 0.4*timer));
    //float g = power*voronoiNoise(vec3(interp_UV*frequency, -0.7*timer));
    //float b = power*voronoiNoise(vec3(interp_UV*frequency, 0.8*timer));

    vec3 color = voronoiDiagram(vec3(interp_UV*frequency, timer));
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = GGXFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
    /*vec2 uv = interp_UV;
    vec2 uv0 = uv;
    vec3 finalColor = vec3(0.0);
    
    for (float i = 0.0; i < 4.0; i++) {
        uv = fract(uv * 1.5) - 0.5;

        float d = length(uv) * exp(-length(uv0));

        vec3 col = palette(length(uv0) + i * 0.4 + timer * 0.4);

        d = sin(d*8. + timer)/8.;
        d = abs(d);

        d = pow(0.01 / d, 1.2);

        finalColor += col * d;
    }

    float shadow = Shadow();

    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        finalColor = paint;
    }
        
    return vec4(finalColor, 1.0);*/
}

subroutine(fragShaders) vec4 StripesSmoothstepPlusGGX() 
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

    color = GGXFunc(color);
    float shadow = Shadow();

    return vec4((1.1 - shadow) * color,1.0);
}

subroutine(fragShaders) vec4 CirclesSmoothstepPlusGGX() 
{
    vec2 k = fract(interp_UV * 5.0);
    float f = smoothstep(0.3, 0.32, length(k-0.5));
    
    vec3 color = mix(colorIn, vec3(0.0,0.0,0.0), f);
    vec3 paint = getMeshColor();

    if (paint.r + paint.g + paint.b > 0.0)
    {
        color = paint;
    }

    color = GGXFunc(color);
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


    vec3 lambert = (Kd*diffColor);


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


        color += (lambert + specular) * NdotL;
    }
    return color;
}


vec3 palette( float t ) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263,0.416,0.557);

    return a + b*cos( 6.28318*(c*t+d) );
}



// Hash function
vec2 hash(vec2 p)
{
    float x = dot(p, vec2(123.4, 234.5));
    float y = dot(p, vec2(345.6, 456.7));

    vec2 noise = vec2(x,y);
    noise = sin(noise) * 43758.5453;
    return fract(noise);
}

float voronoiNoise(vec3 p)
{
    vec2 cell = floor(p.xy);
    vec2 uvw = fract(p.xy);
    
    float minDist = 1.0; // Minimum distance initialization
    
    for ( int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 cellShift = vec2(float(x), float(y));

            vec2 neighborPoint = sin(p.z * hash(cellShift + cell)) * 0.5;

            vec2 diff = cellShift + neighborPoint - uvw;

            float d = dot(diff, diff);

            minDist = min(d, minDist);

        }
    }
    
    // Return the Voronoi noise value
    return minDist;
}

vec3 voronoiDiagram(vec3 p)
{
    vec2 cell = floor(p.xy);
    vec2 uvw = fract(p.xy);
    
    float minDist = 1.0; // Minimum distance initialization

    vec2 finalCell = vec2(0.0,0.0);
    
    for ( int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 cellShift = vec2(float(x), float(y));

            vec2 neighborPoint = sin(p.z * hash(cellShift + cell)) * 0.5;

            vec2 diff = cellShift + neighborPoint - uvw;

            float d = dot(diff, diff);

            if (d < minDist) 
            {
                minDist = d;
                finalCell = cell + cellShift;
            }

        }
    }
    
    return setColors(finalCell);
   
}

vec3 setColors(vec2 cell)
{
    vec3 colors[16] = vec3[](vec3(11.0, 57.0, 84.0)/255.0, vec3(38.0, 84.0, 110.0)/255.0, vec3(182.0, 214.0, 204.0)/255.0, vec3(248.0, 156.0, 115.0)/255.0,
                    vec3(255.0, 58.0, 32.0)/255.0, vec3(245.0, 205.0, 157.0)/255.0, vec3(64.0, 111.0, 136.0)/255.0, vec3(247.0, 181.0, 136.0)/255.0,
                    vec3(116.0, 164.0, 188.0)/255.0, vec3(241.0, 254.0, 198.0)/255.0, vec3(8.0, 126.0, 139.0)/255.0, vec3(200.0, 29.0, 37.0)/255.0,
                    vec3(223.0, 153.0, 165.0)/255.0, vec3(239.0, 122.0, 130.0)/255.0, vec3(20.0, 50.0, 57.0)/255.0, vec3(207.0, 184.0, 200.0)/255.0);
        
    return colors[int(mod(cell.x + cell.y, 16.0))];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////