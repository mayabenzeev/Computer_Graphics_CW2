#ifndef SET_SHADERS_HPP
#define SET_SHADERS_HPP

#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

// set uniforms for textured objects
void set_shader_uniforms( GLuint aShaderID, const Mat44f& aProjCameraWorld, const Mat33f& aNormalMatrix, GLuint aTextureID );
// set uniforms for non-textured objects
void set_shader_uniforms( GLuint aShaderID, const Mat44f& aProjCameraWorld, const Mat33f& aNormalMatrix, const Vec3f& aMaterialColor);
#endif // SET_SHADERS_HPP