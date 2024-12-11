#version 430

// Inputs
in vec3 v2fColor;    // Interpolated vertex color
in vec3 v2fNormal;   // Interpolated normal vector
in vec2 v2fTexCoord;  // Interpolated texture coordinates

layout( binding = 0 ) uniform sampler2D uTexture2D;

// Uniforms
layout( location = 2 ) uniform vec3 uLightDir; // light direction
layout( location = 3 ) uniform vec3 uLightDiffuse;
layout( location = 4 ) uniform vec3 uSceneAmbient;


// Outputs
out vec4 oColor;

void main() 
{
    vec3 normal = normalize(v2fNormal);
    float nDotL = max( 0.0, dot( normal, uLightDir ) );
    vec3 color = (uSceneAmbient + nDotL * uLightDiffuse) * v2fColor;
    vec3 textureColor = texture( uTexture2D, v2fTexCoord ).rgb;
    oColor = vec4(color * textureColor, 1.0);
}


