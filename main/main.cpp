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

#include <iostream>


namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";
	// ************

	constexpr float kMovementPerSecond_ = 5.0f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel
	struct State_
	{
		ShaderProgram* prog;
		
		struct CamCtrl_
		{
			bool cameraActive;
			bool isForward, isBackward, isLeft, isRight, isUp, isDown;
			bool actionSpeedUp, actionSlowDown;
			
			float phi, theta;
			float radius;
			float movementSpeed = kMovementPerSecond_;
			
			Vec3f cameraPosition = Vec3f(0.0f, 0.0f, 3.0f); // Starting position
			Vec3f cameraForwardDirection = Vec3f(0.0f, 0.0f, -1.0f); // Initially facing along -Z
			Vec3f cameraUpDirection = Vec3f(0.0f, 1.0f, 0.0f); // Y is up
			Vec3f cameraRightDirection = Vec3f(1.0f, 0.0f, 0.0f); // X is right
			Vec3f targetPosition;

			float lastX, lastY;
		} camControl;
	};

	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	// *********
	void glfw_callback_motion_( GLFWwindow*, double, double );
	void glfw_callback_mouse_button_( GLFWwindow*, int, int, int );
	void update_camera_position( State_::CamCtrl_&, float );
	void update_camera_direction_vectors( State_::CamCtrl_&);

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
	glfwSetMouseButtonCallback(window, &glfw_callback_mouse_button_);

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

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: global GL setup goes here
	// ******************
	glEnable( GL_FRAMEBUFFER_SRGB ); // enables automatic sRGB conversion of colors
	glEnable( GL_CULL_FACE ); // Enable face culling
	// glFrontFace(GL_CCW); 
	glEnable( GL_DEPTH_TEST ); // Enable depth testing
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f ); // Sets the clear color to dark gray 
	// ******************

	OGL_CHECKPOINT_ALWAYS();


	// ************
	// Load Mesh
	SimpleMeshData objMeshResult = load_wavefront_obj("assets/cw2/langerso.obj");
	GLuint vao = create_vao(objMeshResult); // Returns a VAO pointer from the Attributes object
	std::size_t numVertices = objMeshResult.positions.size() ; // Calculate the number of vertices to draw later


	auto last = Clock::now();
	// ************
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
				// This is a bit of anglea hack.
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
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;

		// Update camera state
		update_camera_position(state.camControl, dt);
        update_camera_direction_vectors(state.camControl);

		// Update: compute transformation matrices
		// Define "View Matrix" (model2world) - transformation in world space 
		Mat44f model2world = make_rotation_y(0.f); // Rotation around the Y axis 

		// Define the camera rotation matrices
		Mat44f Rx = make_rotation_x(state.camControl.theta); // Theta controls vertical rotation
		Mat44f Ry = make_rotation_y(state.camControl.phi); // Phi controls the horizontal rotation
		// Define the camera position in world space
		Mat44f T = make_translation({ 0.f, 0.f, -state.camControl.radius });
		// Create world to camera matrix by first translating and then rotating
		Mat44f world2camera = T * Rx * Ry;

		// Define "Projection Matrix" - Create 3D perspective in the camera's 2D view
		Mat44f projection = make_perspective_projection( 60.f * std::numbers::pi_v<float> / 180.f, fbwidth/float(fbheight), 0.1f, 100.0f );

		// Define final transformation matrix - projCameraWorld matrix
		Mat44f projCameraWorld = projection * world2camera * model2world;

		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );
		// ************** from ex4

		// Draw scene
		OGL_CHECKPOINT_DEBUG();
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  // Resets also the depth values to the farthest dept 
		
		// Use shader program
		glUseProgram( prog.programId() );
		// set tranformation and normal matrices as uniforms to the shaders
		glUniformMatrix4fv( 0, 1, GL_TRUE, projCameraWorld.v );	// projCameraWorld matrix 
		glUniformMatrix3fv( 1, 1, GL_TRUE, normalMatrix.v); // Normal matrix
		
		// Set lightning t o the shaders
		Vec3f lightDir = normalize( Vec3f{ -1.f, 1.f, 0.5f } );
		glUniform3fv( 2, 1, &lightDir.x );
		// glUniform3f( 3, 0.9f, 0.9f, 0.6f ); // Yellow model
		glUniform3f( 3, 1.f, 1.f, 1.f );  // White model
		// glUniform3f( 4, 0.2f, 0.2f, 0.2f );  // Lighter Model
		glUniform3f( 4, 0.05f, 0.05f, 0.05f );  // Darker Model

		// Draw scene
		glBindVertexArray( vao ); // Pass source input as defined in our VAO
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glDrawArrays( GL_TRIANGLES, 0, numVertices ); // Draw <numVertices> vertices , starting at index 0
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

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int aMods)
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

			// Camera controls if camera is active
			if( state->camControl.cameraActive )
			{
				std::cout << "Key pressed: " << aKey << " Action: " << aAction << std::endl;
				if (GLFW_KEY_W == aKey)
				{
					state->camControl.isForward = (GLFW_PRESS == aAction);
					std::cout << "isForward: " << state->camControl.isForward << std::endl;

				}
				else if (GLFW_KEY_S == aKey)
				{
					state->camControl.isBackward = (GLFW_PRESS == aAction);
				}
				else if (GLFW_KEY_D == aKey)
				{
					state->camControl.isRight = (GLFW_PRESS == aAction);
				}
				else if (GLFW_KEY_A == aKey)
				{
					state->camControl.isLeft = (GLFW_PRESS == aAction);
				}
				else if (GLFW_KEY_Q == aKey)
				{
					state->camControl.isDown = (GLFW_PRESS == aAction);
				}
				else if (GLFW_KEY_E == aKey)
				{
					state->camControl.isUp = (GLFW_PRESS == aAction);
				}

				// Controlling the speed
				if ( GLFW_MOD_SHIFT == aMods ) 
				{
					if( GLFW_PRESS == aAction )
						state->camControl.actionSpeedUp = true;
					else if( GLFW_RELEASE == aAction )
						state->camControl.actionSpeedUp = false;
        		} 
				else if ( GLFW_MOD_CONTROL == aMods ) 
				{
            		if( GLFW_PRESS == aAction )
						state->camControl.actionSlowDown = true;
					else if( GLFW_RELEASE == aAction )
						state->camControl.actionSlowDown = false;
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

	// ***********
	void glfw_callback_mouse_button_( GLFWwindow* aWindow, int aButton, int aAction, int aMods )
	{
		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			if ( GLFW_MOUSE_BUTTON_RIGHT == aButton && GLFW_PRESS == aAction )
			{
				state->camControl.cameraActive = !state->camControl.cameraActive;
				std::cout << "Camera Active: " << state->camControl.cameraActive << std::endl;

				if( state->camControl.cameraActive )
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
				else
					glfwSetInputMode( aWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
			}
		}
	}

	void update_camera_position( State_::CamCtrl_ &aCamControl, float dt)
	{
		if (!aCamControl.cameraActive) return; // No movement if the camera is inactive
    
		// float movementSpeed = aCamControl.movementSpeed;

		// Adjust Camera speeds
		if ( aCamControl.actionSpeedUp )
		{
			aCamControl.movementSpeed *= 2.f; //TODO: Understand the movements
			if (aCamControl.movementSpeed < 0.1f) aCamControl.movementSpeed = 0.1f; // Prevent speed from going to 0
		}
		else if ( aCamControl.actionSlowDown )
		{
			aCamControl.movementSpeed *= 0.5f;
			if (aCamControl.movementSpeed < 0.1f) aCamControl.movementSpeed = 0.1f; // Prevent speed from going to 0

		}
		std::cout << "Movement speed: " << aCamControl.movementSpeed << std::endl;

		// Adjust camera position
		if ( aCamControl.isForward ){

			aCamControl.cameraPosition += aCamControl.cameraForwardDirection * aCamControl.movementSpeed * dt;
			std::cout << "Camera Position: " << aCamControl.cameraPosition.x << std::endl;
			std::cout << "cameraForwardDirection: "
					<< aCamControl.cameraForwardDirection.x << ", "
					<< aCamControl.cameraForwardDirection.y << ", "
					<< aCamControl.cameraForwardDirection.z
					<< std::endl;
			std::cout << "Camera movementSpeed: " << aCamControl.movementSpeed << std::endl;
			std::cout << "dt: " << dt << std::endl;

		}
		if ( aCamControl.isBackward )
			aCamControl.cameraPosition -= aCamControl.cameraForwardDirection * aCamControl.movementSpeed * dt;
		if (aCamControl.isRight)
   			aCamControl.cameraPosition += aCamControl.cameraRightDirection * aCamControl.movementSpeed * dt;
		if (aCamControl.isLeft)
    		aCamControl.cameraPosition -= aCamControl.cameraRightDirection * aCamControl.movementSpeed * dt;
		if (aCamControl.isUp)
			aCamControl.cameraPosition += aCamControl.cameraUpDirection * aCamControl.movementSpeed * dt;
		if (aCamControl.isDown)
			aCamControl.cameraPosition -= aCamControl.cameraUpDirection * aCamControl.movementSpeed * dt;
		// if ( state.camControl.radius <= 0.1f )
		// 	state.camControl.radius = 0.1f;
	}

	void update_camera_direction_vectors( State_::CamCtrl_ &aCamControl)
	{
		// Calculate the target position
		Vec3f targetPosition = aCamControl.cameraPosition + aCamControl.cameraForwardDirection * aCamControl.radius;

		Vec3f newForward = normalize(targetPosition - aCamControl.cameraPosition);
		Vec3f newRight = normalize(cross(aCamControl.cameraUpDirection, newForward));
		Vec3f newUp = normalize(cross(newForward, newRight));

		// Update the camera vectors
		aCamControl.cameraForwardDirection = newForward;
		aCamControl.cameraRightDirection = newRight;
		aCamControl.cameraUpDirection = newUp;
	}
	// ***********
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
