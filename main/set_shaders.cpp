#include <glad/glad.h>

#include "set_shaders.hpp"

void set_shader_uniforms( GLuint aShaderID, const Mat44f& aProjCameraWorld, const Mat33f& aNormalMatrix, GLuint aTextureID ) 
{
    glUseProgram( aShaderID );
    glUniformMatrix4fv( 0, 1, GL_TRUE, aProjCameraWorld.v );
    glUniformMatrix3fv( 1, 1, GL_TRUE, aNormalMatrix.v );

    // Lighting
    Vec3f lightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );
    glUniform3fv( 2, 1, &lightDir.x );
    glUniform3f( 3, 1.f, 1.f, 1.f );  // White light model
    glUniform3f( 4, 0.05f, 0.05f, 0.05f );  

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, aTextureID );
    // glUniform1i( 5, 0 );  // Set texture unit 0 as the active texture
}

void set_shader_uniforms( GLuint aShaderID, const Mat44f& aProjCameraWorld, const Mat33f& aNormalMatrix, const Vec3f& aMaterialColor)
{
    glUseProgram( aShaderID );
    glUniformMatrix4fv( 0, 1, GL_TRUE, aProjCameraWorld.v );
    glUniformMatrix3fv( 1, 1, GL_TRUE, aNormalMatrix.v );

    // Lighting 
    Vec3f lightDir = normalize( Vec3f{ 0.f, 1.f, -1.f } );
    glUniform3fv( 2, 1, &lightDir.x );
    glUniform3f( 3, 1.f, 1.f, 1.f );  // White light model
    glUniform3f( 4, 0.05f, 0.05f, 0.05f );  
    glUniform3fv( 5, 1, &aMaterialColor.x ); // Set material color
}
