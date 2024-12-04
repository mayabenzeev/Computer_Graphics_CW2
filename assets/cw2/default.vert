#version 430

// Inputs
layout( location = 0 ) in vec3 iPosition;  // Vertex position
layout( location = 1 ) in vec3 iColor;     // Vertex color
//layout( location = 2 ) in vec3 iNormal;    // Vertex normal
//layout( location = 3 ) in vec2 iTexCoord; // Texture coordinates

// Uniforms
layout( location = 0 ) uniform mat4 uModelViewProjection;
//layout( location = 1 ) uniform mat3 uNormalMatrix;

// Outputs
out vec3 v2fColor;    
//out vec3 v2fNormal;    
//out vec2 v2fTexCoord;

void main() {
    // Transform the input vertex position into clip space
    gl_Position = uModelViewProjection * vec4(iPosition.xyz, 1.0);

    // Pass the color, normal, and texture coordinates to the fragment shader
    v2fColor = iColor;
    //v2fNormal = normalize(uNormalMatrix * iNormal);
    //v2fTexCoord = iTexCoord;
}
