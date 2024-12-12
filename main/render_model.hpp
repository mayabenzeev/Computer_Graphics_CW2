#ifndef RENDER_MODEL_HPP
#define RENDER_MODEL_HPP

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"
#include "../support/program.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// set uniforms for textured objects
void set_shader_uniforms( GLuint aShaderID, const Mat44f& aProjCameraWorld, const Mat33f& aNormalMatrix, GLuint aTextureID );
// set uniforms for non-textured objects
void set_shader_uniforms( GLuint aShaderID, const Mat44f& aProjCameraWorld, const Mat33f& aNormalMatrix, const Vec3f& aMaterialColor );
// render model for textured objects
void render_model( ShaderProgram& aShaderProg, GLuint aVAO, const Mat44f& aProjection, const Mat44f& aWorld2camera, const Vec3f& aPosition, GLuint aTextureID, std::size_t aNumVertices );
// render model for non-textured objects
void render_model( ShaderProgram& aShaderProg, GLuint aVAO, const Mat44f& aProjection, const Mat44f& aWorld2camera, const Vec3f& aPosition, std::size_t aNumVertices );

#endif // RENDER_MODEL_HPP