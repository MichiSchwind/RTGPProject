#version 410 core

uniform sampler2D paintTexture;
uniform sampler2D bakeTexture;
uniform sampler2D bakeDepthMap;


in vec2 interp_UV;
in vec4 sPos;

out vec4 fragmentColor;

void main() {
    // we transform the coordinates of sPos from [-1,1] to [0,1]
    vec3 screenPos = 0.5 * (1.0 + (sPos.xyz/sPos.w));


    // Sample the paint strokes texture
    vec3 paintColor = texture(paintTexture, screenPos.xy).rgb;

    // Sample the mesh texture
    vec3 meshColor = texture(bakeTexture, interp_UV).rgb;

    // sample the depth of the vertex 
    float depth = texture(bakeDepthMap, screenPos.xy).r;
    float currentDepth = screenPos.z;

    // overwrite the meshcolor with paintColor if the paintColor is not black and the depth of the vertex is the nearest to the camera
    // this prevents coloring through the mesh 
    if (paintColor.r + paintColor.g + paintColor.b > 0  && (currentDepth - 0.00005 < depth))
    {
        meshColor = paintColor;
    }

    fragmentColor = vec4(meshColor, 1.0);

}