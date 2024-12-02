#version 430

// Input attributes from the VBO
layout(location = 0) in vec3 iPosition;  // Vertex position
layout(location = 1) in vec3 iNormal;    // Vertex normal
layout(location = 2) in vec2 iTexCoord; // Texture coordinates

// Uniform matrix (combined projection, view, and model matrix)
layout(location = 3) uniform mat4 uProjCameraWorld;

// Outputs to the fragment shader
out vec3 vNormal;    // Interpolated normal vector
out vec2 vTexCoord;  // Interpolated texture coordinates

void main() {
    // Transform the input vertex position into clip space
    gl_Position = uProjCameraWorld * vec4(iPosition, 1.0);

    // Pass the normal and texture coordinates to the fragment shader
    vNormal = iNormal;
    vTexCoord = iTexCoord;
}
