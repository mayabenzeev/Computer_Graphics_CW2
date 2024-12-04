#version 430

// Input attributes from the VBO
layout(location = 0) in vec3 iPosition;  // Vertex position
layout(location = 1) in vec3 iColor;     // Vertex color
layout(location = 2) in vec2 iTexCoord; // Texture coordinates
layout(location = 3) in vec3 iNormal;    // Vertex normal

// Uniform matrix (combined projection, view, and model matrix)
layout(location = 0) uniform mat4 uModelViewProjection;

// Outputs to the fragment shader
out vec3 vColor;    // Interpolated color
out vec3 vNormal;    // Interpolated normal
out vec2 vTexCoord;  // Interpolated texture coordinates

void main() {
    // Transform the input vertex position into clip space
    gl_Position = uModelViewProjection * vec4(iPosition.xyz, 1.0);

    // Pass the color, normal, and texture coordinates to the fragment shader
    vColor = iColor;
    vNormal = iNormal;
    vTexCoord = iTexCoord;
}
