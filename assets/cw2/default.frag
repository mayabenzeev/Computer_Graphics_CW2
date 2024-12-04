#version 430

// Inputs
in vec3 v2fColor;    // Interpolated vertex color
in vec3 v2fNormal;   // Interpolated normal vector
in vec2 v2fTexCoord;  // Interpolated texture coordinates

// Uniforms
layout(location = 2) uniform vec3 uBaseColor; // Base color of the object

// Outputs
out vec4 oColor;

void main() {
    vec3 normal = normalize(v2fNormal);
    oColor = vec4(normal.xyz, 1.0);
}


