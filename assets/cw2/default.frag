#version 430

// Inputs
in vec3 v2fColor;    // Interpolated vertex color
in vec3 v2fNormal;   // Interpolated normal vector
//in vec2 v2fTexCoord;  // Interpolated texture coordinates

// Uniforms
//layout( location = 2 ) uniform vec3 uBaseColor; // Base color of the object
layout( location = 2 ) uniform vec3 uLightDir; // light direction
layout( location = 3 ) uniform vec3 uLightDiffuse; //
layout( location = 4 ) uniform vec3 uSceneAmbient;

// Outputs
out vec3 oColor;

void main() {
    //oColor = uBaseColor * v2fColor;

    vec3 normal = normalize(v2fNormal);
    
    float nDotL = max( 0.0, dot( normal, uLightDir ) );
    oColor = (uSceneAmbient + nDotL * uLightDiffuse) * v2fColor;
}


