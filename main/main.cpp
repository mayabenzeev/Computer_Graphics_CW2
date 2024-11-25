#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "../third_party/rapidobj/include/rapidobj/rapidobj.hpp"

#include "defaults.hpp"


namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";
	// ************
	struct State_
	{
		ShaderProgram* prog;
	};

	rapidobj::Result load_wavefront_obj( char const* aPath );

	GLuint createVAO( const rapidobj::Result &result ) ; 

	// ************

	
	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};

}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	GLFWwindow* window = glfwCreateWindow(
		1280,
		720,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };


	// Set up event handling
	// ************
	State_ state{};

	glfwSetWindowUserPointer( window, &state );
	// **********
	// TODO: Additional event handling setup

	glfwSetKeyCallback( window, &glfw_callback_key_ );

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// ************
	// Load shader program
	ShaderProgram prog( {
		{ GL_VERTEX_SHADER, "assets/cw2/default.vert" },
		{ GL_FRAGMENT_SHADER, "assets/cw2/default.frag" }
	} );

	state.prog = &prog;
	// ************

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();


	// ************
	//TODO: VAO VBO
	rapidobj::Result objResult = load_wavefront_obj("assets/cw2/langerso.obj");
	GLuint vao = createVAO(objResult); // Returns a VAO pointer from the Attributes object
	// ************

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}

			glViewport( 0, 0, nwidth, nheight );
		}

		// Update state
		//TODO: update state

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		// ************
		//TODO: draw frame
		// Clear color buffer to specified clear color (glClearColor())
		glClear( GL_COLOR_BUFFER_BIT );
		// We want to draw with our program.
		glUseProgram( prog.programId() );
		// Specify the base color (uBaseColor in location 0 in the fragment shader)
		static float const baseColor[] = { 0.2f, 1.f, 1.f };
		glUniform3fv( 0, 1, baseColor ); /*first item is location in shader*/
		// Source input as defined in our VAO
		glBindVertexArray( vao );
		// Draw a single triangle (= 3 vertices), starting at index 0
		glDrawArrays( GL_TRIANGLES, 0, 3 );
		// Reset state
		glBindVertexArray( 0 );
		glUseProgram( 0 );
		// ************

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	//TODO: additional cleanup
	
	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::fprintf( stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}
	}

	rapidobj::Result load_wavefront_obj( char const* aPath )
	{
		// Ask rapidobj to load the requested file
		auto result = rapidobj::ParseFile( aPath );
		if( result.error ) throw Error( "Unable to load OBJ file ’%s’: %s", aPath, result.error.code.message().c_str() );

		// OBJ files can define faces that are not triangles. However, OpenGL will only render triangles (and lines
		// and points), so we must triangulate any faces that are not already triangles. Fortunately, rapidobj can do
		// this for us.
		rapidobj::Triangulate( result );

		return result;
	}

	GLuint createVAO( const rapidobj::Result &result ) 
	{
		GLuint vboPositions, vboTexcoords, vboNormals;
		GLuint vao = 0;

		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );

		if ( !result.attributes.positions.empty() ) 
		{
			glGenBuffers( 1, &vboPositions );
			glBindBuffer( GL_ARRAY_BUFFER, vboPositions );
			glBufferData( GL_ARRAY_BUFFER, sizeof(float) * result.attributes.positions.size(), result.attributes.positions.data(), GL_STATIC_DRAW) ;
			glVertexAttribPointer(
				0, // location = 0 in vertex shader
				3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
				0, // stride = 0 indicates that there is no padding between inputs
				0 // data starts at offset 0 in the VBO.
			);
			glEnableVertexAttribArray( 0 );
		}

		if ( !result.attributes.texcoords.empty() ) 
		{
			glGenBuffers( 1, &vboTexcoords );
			glBindBuffer( GL_ARRAY_BUFFER, vboTexcoords) ;
			glBufferData( GL_ARRAY_BUFFER, sizeof(float) * result.attributes.texcoords.size(), result.attributes.texcoords.data(), GL_STATIC_DRAW );
				glVertexAttribPointer(
				1, // location = 0 in vertex shader
				2, GL_FLOAT, GL_FALSE, // 2 floats, not normalized to [0..1] (GL FALSE)
				0, // stride = 0 indicates that there is no padding between inputs
				0 // data starts at offset 0 in the VBO.
			);
			glEnableVertexAttribArray( 1 );
		}

		if ( !result.attributes.normals.empty() ) 
		{
			glGenBuffers( 1, &vboNormals );
			glBindBuffer( GL_ARRAY_BUFFER, vboNormals );
			glBufferData( GL_ARRAY_BUFFER, sizeof(float) * result.attributes.normals.size(), result.attributes.normals.data(), GL_STATIC_DRAW );
				glVertexAttribPointer(
				2, // location = 0 in vertex shader
				3, GL_FLOAT, GL_FALSE, // 3 floats, not normalized to [0..1] (GL FALSE)
				0, // stride = 0 indicates that there is no padding between inputs
				0 // data starts at offset 0 in the VBO.
			);
			glEnableVertexAttribArray( 2 );
		}

		// unbind vao
		glBindVertexArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		// Clean up buffers. these are not deleted fully, as the VAO holds a reference to them.
		glDeleteBuffers( 1, &vboPositions );
		glDeleteBuffers( 1, &vboTexcoords );
		glDeleteBuffers( 1, &vboNormals );

		return vao;
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}
