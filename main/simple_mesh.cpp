#include "simple_mesh.hpp"

SimpleMeshData concatenate( SimpleMeshData aM, SimpleMeshData const& aN )
{
	aM.positions.insert( aM.positions.end(), aN.positions.begin(), aN.positions.end() );
	aM.colors.insert( aM.colors.end(), aN.colors.begin(), aN.colors.end() );
	aM.normals.insert( aM.normals.end(), aN.normals.begin(), aN.normals.end() );
	return aM;
}


GLuint create_vao( SimpleMeshData const& aMeshData )
{
	GLuint vboPositions, vboColors, vboNormals;
	// , vboTexcoords, vboNormals;
	
	GLuint vao = 0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	// std::vector<float> vertexColors;

	// Positions VBO
	glGenBuffers( 1, &vboPositions ); // Generates 1 name for vbo positions
	glBindBuffer( GL_ARRAY_BUFFER, vboPositions );
	glBufferData( GL_ARRAY_BUFFER, sizeof(Vec3f) * aMeshData.positions.size(), aMeshData.positions.data(), GL_STATIC_DRAW) ; // Allocate and store data
	glVertexAttribPointer(
		0, // location = 0 in vertex shader
		3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
		0, // stride = 0 indicates that there is no padding between inputs
		0 // data starts at offset 0 in the VBO.
	);
	glEnableVertexAttribArray( 0 );
	
	
	// Colours VBO
	glGenBuffers( 1, &vboColors ); // Generates 1 name for vbo colours
	glBindBuffer( GL_ARRAY_BUFFER, vboColors );
	glBufferData( GL_ARRAY_BUFFER, sizeof(Vec3f) * aMeshData.colors.size(), aMeshData.colors.data(), GL_STATIC_DRAW) ; // Allocate and store data
	glVertexAttribPointer(
		1, // location = 1 in vertex shader
		3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
		0, // stride = 0 indicates that there is no padding between inputs
		0 // data starts at offset 0 in the VBO.
	);
	glEnableVertexAttribArray( 1 );


	glGenBuffers( 1, &vboNormals ); // Generates 1 name for vbo normals
	glBindBuffer( GL_ARRAY_BUFFER, vboNormals );
	glBufferData( GL_ARRAY_BUFFER, sizeof(Vec3f) * aMeshData.normals.size(), aMeshData.normals.data(), GL_STATIC_DRAW ); // Allocate and store data
		glVertexAttribPointer(
		2, // location = 2 in vertex shader
		3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
		0, // stride = 0 indicates that there is no padding between inputs
		0 // data starts at offset 0 in the VBO.
	);
	glEnableVertexAttribArray( 2 );


	// if ( !result.attributes.texcoords.empty() ) 
	// {
	// 	glGenBuffers( 1, &vboTexcoords ); // Generates 1 name for vbo textures
	// 	glBindBuffer( GL_ARRAY_BUFFER, vboTexcoords) ;
	// 	glBufferData( GL_ARRAY_BUFFER, sizeof(float) * result.attributes.texcoords.size(), result.attributes.texcoords.data(), GL_STATIC_DRAW ); // Allocate and store data
	// 		glVertexAttribPointer(
	// 		3, // location = 3 in vertex shader
	// 		2, GL_FLOAT, GL_FALSE, // 2 floats, not normalized to [0..1] (GL FALSE)
	// 		0, // stride = 0 indicates that there is no padding between inputs
	// 		0 // data starts at offset 0 in the VBO.
	// 	);
	// 	glEnableVertexAttribArray( 3 );
	// }



	// unbind vao
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Clean up buffers. these are not deleted fully, as the VAO holds a reference to them.
	glDeleteBuffers( 1, &vboPositions );
	glDeleteBuffers( 1, &vboColors );
	glDeleteBuffers( 1, &vboNormals );
	// glDeleteBuffers( 1, &vboTexcoords );

	return vao;
}


