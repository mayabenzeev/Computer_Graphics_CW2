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
#include "load_texture.hpp"
#include "set_shaders.hpp"

#include <iostream>


namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	constexpr float kMovementPerSecond_ = 5.0f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel
	constexpr Vec3f kLandpadPosition1_ = {5.f, 0.f, -5.f}; // Placed on the sea - y-axis is 0
	constexpr Vec3f kLandpadPosition2_ = {-2.1f, 0.f, 1.1f};
	constexpr Vec3f kLandpadColor_ = {1.f, 1.f, 1.f};

	struct State_
	{
		ShaderProgram* progTexture;
		ShaderProgram* progNonTexture;
		
		struct CamCtrl_
		{
			bool cameraActive;
			bool isForward, isBackward, isLeft, isRight, isUp, isDown;
			bool actionSpeedUp, actionSlowDown;
			
			float phi, theta;
			// float radius;
			float movementSpeed = kMovementPerSecond_;
			
			Vec3f cameraPosition = Vec3f{0.0f, -0.2f, 0.0f}; // Starting position
			Vec3f cameraForwardDirection = Vec3f{0.0f, 0.0f, -1.0f}; // Initially facing along -Z
			Vec3f cameraUpDirection = Vec3f{0.0f, 1.0f, 0.0f}; // Y is up
			Vec3f cameraRightDirection = Vec3f{1.0f, 0.0f, 0.0f}; // X is right
			Vec3f targetPosition;

			float lastX, lastY;
		} camControl;
	};

	void glfw_callback_error_( int, char const* );
	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	void glfw_callback_motion_( GLFWwindow*, double, double );
	void glfw_callback_mouse_button_( GLFWwindow*, int, int, int );
	void update_camera_position( State_::CamCtrl_&, float );
	void update_camera_direction_vectors( State_::CamCtrl_&);

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
	State_ state{};
	glfwSetWindowUserPointer( window, &state );
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

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );
	glViewport( 0, 0, iwidth, iheight );

	// Animation state
	auto last = Clock::now();
	
	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	// global GL setup 
	glEnable( GL_FRAMEBUFFER_SRGB ); // enables automatic sRGB conversion of colors
	glEnable( GL_CULL_FACE ); // Enable face culling
	// glFrontFace(GL_CCW); 
	glEnable( GL_DEPTH_TEST ); // Enable depth testing
	glClearColor( 0.2f, 0.2f, 0.2f, 0.0f ); // Sets the clear color to dark gray 

	OGL_CHECKPOINT_ALWAYS();

	// Load shader program
	ShaderProgram progTexture( {{ GL_VERTEX_SHADER, "assets/cw2/default.vert" }, { GL_FRAGMENT_SHADER, "assets/cw2/textured_objects.frag" }} );
	ShaderProgram progNonTexture( {{ GL_VERTEX_SHADER, "assets/cw2/default.vert" }, { GL_FRAGMENT_SHADER, "assets/cw2/non_textured_objects.frag" }} );

	// Load Meshes and Textures
	
	state.progTexture = &progTexture;
	state.progNonTexture = &progNonTexture;
	// state.camControl.radius = 10.f;

	SimpleMeshData langersoMesh = load_wavefront_obj("assets/cw2/langerso.obj"); // Load Mesh
	GLuint langersoVAO = create_vao(langersoMesh); // Returns a VAO pointer from the Attributes object
	std::size_t langersoVertices = langersoMesh.positions.size() ; // Calculate the number of vertices to draw later

	SimpleMeshData landingpadMesh = load_wavefront_obj("assets/cw2/landingpad.obj"); // Load Mesh
	GLuint landingpadVAO = create_vao(landingpadMesh); // Returns a VAO pointer from the Attributes object
	std::size_t landingpadVertices = landingpadMesh.positions.size() ; // Calculate the number of vertices to draw later
	
	GLuint textureID = load_texture_2d("assets/cw2/L3211E-4k.jpg");  // Load Texture

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

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );  // Resets also the depth values to the farthest dept 

		// Update state
		auto const now = Clock::now();
		float dt = std::chrono::duration_cast<Secondsf>(now-last).count();
		last = now;

		// angle += dt * std::numbers::pi_v<float> * 0.3f;
		// if( angle >= 2.f*std::numbers::pi_v<float> )
		// 	angle -= 2.f*std::numbers::pi_v<float>;

		// Update camera state
		update_camera_position(state.camControl, dt);
        update_camera_direction_vectors(state.camControl);

		// Define the camera rotation matrices
		Mat44f Rx = make_rotation_x(state.camControl.theta); // Theta controls vertical rotation
		Mat44f Ry = make_rotation_y(state.camControl.phi); // Phi controls the horizontal rotation		

		// transformation for langerso
		Mat44f T = make_translation({state.camControl.cameraPosition.x, state.camControl.cameraPosition.y, -state.camControl.cameraPosition.z}); // Define the camera position in world space
		Mat44f world2camera = Rx * Ry * T; // Create world to camera matrix by first translating and then rotating
		Mat44f projection = make_perspective_projection(  60.f * std::numbers::pi_v<float> / 180.f, fbwidth/float(fbheight), 0.1f, 100.0f );

		// Draw scene
		// Render langerso model
		Mat44f model2world = kIdentity44f; 
		Mat44f projCameraWorld = projection * world2camera * model2world; // Place into camera space
		Mat33f normalMatrix = mat44_to_mat33( transpose(invert(model2world)) );
		set_shader_uniforms( progTexture.programId(), projCameraWorld, normalMatrix, textureID );
		glBindVertexArray( langersoVAO ); // Pass source input as defined in our VAO
		glDrawArrays( GL_TRIANGLES, 0, langersoVertices ); // Draw <numVertices> vertices , starting at index 0

		// Render 1st landingpad
		Mat44f modelMatrix1 = make_translation( kLandpadPosition1_ );
		projCameraWorld = projection * world2camera * modelMatrix1; // Place into camera space
		normalMatrix = mat44_to_mat33( transpose(invert(modelMatrix1)) );
		set_shader_uniforms( progNonTexture.programId(), projCameraWorld, normalMatrix, kLandpadColor_ );
		glBindVertexArray( landingpadVAO ); // Pass source input as defined in our VAO
		glDrawArrays( GL_TRIANGLES, 0, landingpadVertices ) ; // Draw <numVertices> vertices , starting at index 0

		// Render 2nd landingpad
		Mat44f modelMatrix2 = make_translation( kLandpadPosition2_ );
		projCameraWorld = projection * world2camera * modelMatrix2; // Place into camera space
		normalMatrix = mat44_to_mat33( transpose(invert(modelMatrix2)) );
		set_shader_uniforms( progNonTexture.programId(), projCameraWorld, normalMatrix, kLandpadColor_ );
		glDrawArrays( GL_TRIANGLES, 0, landingpadVertices ) ; // Draw <numVertices> vertices , starting at index 0

		OGL_CHECKPOINT_DEBUG();

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		
		// Reset state
		glBindVertexArray( 0 );
		glUseProgram( 0 );	

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.progTexture = nullptr;
	state.progNonTexture = nullptr;
	
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

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int)
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;	
		}

		if( auto* state = static_cast<State_*>(glfwGetWindowUserPointer( aWindow )) )
		{
			// // R-key reloads shaders.
			// if( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			// {
			// 	if( state->prog )
			// 	{
			// 		try
			// 		{
			// 			state->prog->reload();
			// 			std::fprintf( stderr, "Shaders reloaded and recompiled.\n" );
			// 		}
			// 		catch( std::exception const& eErr )
			// 		{
			// 			std::fprintf( stderr, "Error when reloading shader:\n" );
			// 			std::fprintf( stderr, "%s\n", eErr.what() );
			// 			std::fprintf( stderr, "Keeping old shader.\n" );
			// 		}
			// 	}
			// }
	
			// Camera controls if camera is active
			if( state->camControl.cameraActive )
			{	
				if ( GLFW_KEY_W == aKey )
					state->camControl.isForward = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
				else if (GLFW_KEY_S == aKey)
					state->camControl.isBackward = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
				else if (GLFW_KEY_D == aKey)
					state->camControl.isRight = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
				else if (GLFW_KEY_A == aKey)
					state->camControl.isLeft = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
				else if (GLFW_KEY_Q == aKey)
					state->camControl.isDown = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
				else if (GLFW_KEY_E == aKey)
					state->camControl.isUp = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );

				// Controlling the speed
				if ( GLFW_KEY_LEFT_SHIFT == aKey ||  GLFW_KEY_RIGHT_SHIFT == aKey ) 
					state->camControl.actionSpeedUp = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
				else if ( GLFW_KEY_LEFT_CONTROL == aKey ||  GLFW_KEY_RIGHT_CONTROL == aKey ) 
					state->camControl.actionSlowDown = (GLFW_PRESS == aAction || GLFW_REPEAT == aAction );
			
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
}

namespace
{
	void update_camera_position( State_::CamCtrl_ &aCamControl, float dt)
	{
		if (!aCamControl.cameraActive) return; // No movement if the camera is inactive
    
		// Adjust Camera speeds
		if ( aCamControl.actionSpeedUp ) 
			aCamControl.movementSpeed = std::min(aCamControl.movementSpeed * 1.1f, 10.0f); // Prevent speed from going too fast
		else if (aCamControl.actionSlowDown) 
			aCamControl.movementSpeed = std::max(aCamControl.movementSpeed * 0.9f, 0.1f); // Prevent speed from going to 0

		// Adjust camera position
		if ( aCamControl.isForward )
			aCamControl.cameraPosition += aCamControl.cameraForwardDirection * aCamControl.movementSpeed * dt;
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
		aCamControl.cameraForwardDirection = normalize(Vec3f(
			-cos(aCamControl.theta) * sin(aCamControl.phi),
			sin(aCamControl.theta),
			-cos(aCamControl.theta) * cos(aCamControl.phi)
		));

		aCamControl.cameraRightDirection = normalize(Vec3f(
			-sin(aCamControl.phi + std::numbers::pi_v<float> / 2),
			0,
			-cos(aCamControl.phi + std::numbers::pi_v<float> / 2)
		));
	
		aCamControl.cameraUpDirection = normalize(cross(aCamControl.cameraForwardDirection, aCamControl.cameraRightDirection));
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
