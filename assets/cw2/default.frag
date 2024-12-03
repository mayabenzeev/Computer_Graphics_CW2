#version 430

// Inputs from the vertex shader
in vec3 vColor;    // Interpolated vertex color
in vec3 vNormal;    // Interpolated normal vector
in vec2 vTexCoord;  // Interpolated texture coordinates

// Uniforms
layout(location = 1) uniform vec3 uBaseColor; // Base color of the object

// Output fragment color
out vec4 fragColor;

void main() {
    // Simple diffuse shading based on normal direction
    vec3 normal = normalize(vNormal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5)); // Light direction
    float diff = max(dot(normal, lightDir), 0.0);   // Diffuse factor

    // Compute final color using vertex color
    vec3 color = vColor * diff;

    // Output the final color
    fragColor = vec4(color, 1.0);
}
