#version 430

in vec3 v2fColor;    // Interpolated vertex color
in vec3 v2fNormal;   // Interpolated normal vector

layout( location = 2 ) uniform vec3 uLightDir;
layout( location = 3 ) uniform vec3 uLightDiffuse;
layout( location = 4 ) uniform vec3 uSceneAmbient;

out vec4 oColor;

void main() 
{
    vec3 normal = normalize(v2fNormal);
    float nDotL = max(0.0, dot(normal, uLightDir));
    vec3 color = (uSceneAmbient + nDotL * uLightDiffuse) * v2fColor;
    oColor = vec4(color, 1.0); // Apply material color here
}
