#include <glad/glad.h>

#include "render_model.hpp"

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

    if (aTextureID != 0)
    {
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, aTextureID );
    }
}


void render_model( ShaderProgram& aShaderProg, GLuint aVAO, const Mat44f& aProjection, const Mat44f& aWorld2camera, const Vec3f& aPosition, GLuint aTextureID, std::size_t aNumVertices )
{
    Mat44f model2world = make_translation( aPosition ); 
    set_shader_uniforms( 
        aShaderProg.programId(), 
        aProjection * aWorld2camera * model2world, 
        mat44_to_mat33( transpose(invert(model2world)) ), 
        aTextureID );
    glBindVertexArray( aVAO ); // Pass source input as defined in our VAO
    glDrawArrays( GL_TRIANGLES, 0, aNumVertices ); // Draw <numVertices> vertices , starting at index 0
}

