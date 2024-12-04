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
#include "../vmlib/mat33.hpp"

#include "../third_party/rapidobj/include/rapidobj/rapidobj.hpp"

#include "defaults.hpp"
#include "loadobj.hpp"
#include "simple_mesh.hpp"



namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";
	// ************

	constexpr float kMovementPerSecond_ = 5.f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel
	struct State_
	{
		ShaderProgram* prog;
		
		struct CamCtrl_
		{
			bool cameraActive;
			bool actionZoomIn, actionZoomOut;
			bool actionMoveForward, actionMoveBackward, actionMoveLeft, actionMoveRight, actionMoveUp, actionMoveDown;
			
			float phi, theta, movementSpeed;
			Vec3f cameraPosition;
			float radius;

			float lastX, lastY;
		} camControl;
	};

	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	// *********
	void glfw_callback_motion_( GLFWwindow*, double, double );
	// **********

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
	glfwSetCursorPosCallback( window, &glfw_callback_motion_ );


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
	state.camControl.radius = 10.f;

	// Animation state
	auto last = Clock::now();

	float angle = 0.f;
	// ************

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: global GL setup goes here
	// ******************
	glEnable( GL_FRAMEBUFFER_SRGB ); // enables automatic sRGB conversion of colors
	glEnable( GL_CULL_FACE ); // Enable face culling
	// glFrontFace(GL_CCW); 
	glEnable( GL_DEPTH_TEST ); // Enable depth testing
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f ); // Sets the clear color to gray 
	// ******************

	OGL_CHECKPOINT_ALWAYS();


	// ************
	//TODO: VAO VBO

	SimpleMeshData objMeshResult = load_wavefront_obj("assets/cw2/langerso.obj");
	GLuint vao = create_vao(objMeshResult); // Returns a VAO pointer from the Attributes object

	std::size_t numVertices = objMeshResult.positions.size() / 3; // Calculate the number of vertices to draw later

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
		// ************** from ex4
		// auto const now = Clock::now();
		// float dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		// last = now;

		// angle += dt * std::numbers::pi_v<float> * 0.3f;
		// if( angle >= 2.f*std::numbers::pi_v<float> )
		// 	angle -= 2.f*std::numbers::pi_v<float>;

		// // Update camera state
		// if( state.camControl.actionZoomIn )
		// 	state.camControl.radius -= kMovementPerSecond_ * dt;
		// else if( state.camControl.actionZoomOut )
		// 	state.camControl.radius += kMovementPerSecond_ * dt;

		// if( state.camControl.radius <= 0.1f )
		// 	state.camControl.radius = 0.1f;

		// Update: compute transformation matrices
		// Define the model2world matrix
		Mat44f model2world = make_rotation_y(angle); // Angle updates per frame for spinning cube

		// Define the world2camera matrix "arc-ball control set up"
		Mat44f Rx = make_rotation_x(state.camControl.theta);
		Mat44f Ry = make_rotation_y(state.camControl.phi);
		Mat44f T = make_translation({ 0.f, 0.f, -state.camControl.radius });
		Mat44f world2camera = T * Rx * Ry;

		Mat44f projection = make_perspective_projection( 60.f * std::numbers::pi_v<float> / 180.f, fbwidth/float(fbheight), 0.1f, 100.0f );
		//Define and compute projCameraWorld matrix
		Mat44f projCameraWorld = projection * world2camera * model2world;
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );
		// ************** from ex4

		// Draw scene
		OGL_CHECKPOINT_DEBUG();

		// ************
		//TODO: draw frame
		// Clear color buffer to specified clear color (glClearColor())
		// glClear( GL_COLOR_BUFFER_BIT ); // TODO: for basic 2D rendering but insufficient for 3D rendering.
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  // Resets also the depth values to the farthest dept 
		// glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );  // Resets also the depth values to the farthest dept and stencil testing 

		// Use shader program
		glUseProgram( prog.programId() );

		// Specify the base color ana pass it to the shader
		static float const baseColor[] = { 0.2f, 1.f, 1.f };
		glUniform3fv(2, 1, baseColor); // location 2 in shader
		// glUniformMatrix3fv(
		// 	1, 
		// 	1, 
		// 	GL_TRUE, 
		// 	normalMatrix.v
		// 	);

		// Pass transformation matrix to shader
		glUniformMatrix4fv( 0, 1, GL_TRUE, projCameraWorld.v );	

		// Draw scene
		glBindVertexArray( vao ); // Pass source input as defined in our VAO
		glDrawArrays( GL_TRIANGLES, 0, numVertices ); // Draw <numVertices> vertices (=<numVertices> / 3 triangles), starting at index 0
		// Reset state
		glBindVertexArray( 0 );
		glUseProgram( 0 );
		// ************

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.prog = nullptr;
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

		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			// R-key reloads shaders.
			if( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			{
				if( state->prog )
				{
					try
					{
						state->prog->reload();
						std::fprintf( stderr, "Shaders reloaded and recompiled.\n" );
					}
					catch( std::exception const& eErr )
					{
						std::fprintf( stderr, "Error when reloading shader:\n" );
						std::fprintf( stderr, "%s\n", eErr.what() );
						std::fprintf( stderr, "Keeping old shader.\n" );
					}
				}
			}

			// Space toggles camera
			if( GLFW_KEY_SPACE == aKey && GLFW_PRESS == aAction )
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;

				if( state->camControl.cameraActive )
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
				else
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			}

			// Camera controls if camera is active
			if( state->camControl.cameraActive )
			{
				if( GLFW_KEY_W == aKey )
				{
					if( GLFW_PRESS == aAction )
						state->camControl.actionZoomIn = true;
					else if( GLFW_RELEASE == aAction )
						state->camControl.actionZoomIn = false;
				}
				else if( GLFW_KEY_S == aKey )
				{
					if( GLFW_PRESS == aAction )
						state->camControl.actionZoomOut = true;
					else if( GLFW_RELEASE == aAction )
						state->camControl.actionZoomOut = false;
				}
			}
		}
	}

	void glfw_callback_motion_( GLFWwindow* aWindow, double aX, double aY )
	{
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			if( state->camControl.cameraActive )
			{
				auto const dx = float(aX-state->camControl.lastX);
				auto const dy = float(aY-state->camControl.lastY);

				state->camControl.phi += dx*kMouseSensitivity_;
				
				state->camControl.theta += dy*kMouseSensitivity_;
				if( state->camControl.theta > std::numbers::pi_v<float>/2.f )
					state->camControl.theta = std::numbers::pi_v<float>/2.f;
				else if( state->camControl.theta < -std::numbers::pi_v<float>/2.f )
					state->camControl.theta = -std::numbers::pi_v<float>/2.f;
			}

			state->camControl.lastX = float(aX);
			state->camControl.lastY = float(aY);
		}
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
