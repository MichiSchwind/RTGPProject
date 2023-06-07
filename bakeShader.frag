#version 410 core

uniform sampler2D paintTexture;
uniform sampler2D meshTexture;
uniform sampler2D bakeShadowMap;

uniform vec4 brushColor;
uniform vec2 screenScale;

in vec2 vertexTexCoord;
in vec4 cameraPos;

out vec4 fragmentColor;

void main() {
    vec3 screenPos = 0.5 * (1.0 + (cameraPos.xyz/cameraPos.w));


    // Sample the paint strokes texture
    vec3 paintColor = texture(paintTexture, screenPos.xy).rgb;

    // Sample the mesh texture
    vec3 meshColor = texture(meshTexture, vertexTexCoord).rgb;

    float depth = texture(bakeShadowMap, screenPos.xy).r;
    float currentDepth = screenPos.z;


    if (paintColor.r + paintColor.g + paintColor.b > 0  && (currentDepth - 0.0005 < depth))
    {
        meshColor = paintColor;
    }

    fragmentColor = vec4(meshColor, 1.0);

}