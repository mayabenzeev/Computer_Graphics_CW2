#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <numbers>
#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"
#include "../vmlib/mat33.hpp"

#include "../third_party/rapidobj/include/rapidobj/rapidobj.hpp"

#include "defaults.hpp"
#include "load_obj.hpp"
#include "simple_mesh.hpp"
#include "load_texture.hpp"
#include "render_model.hpp"
#include "space_vehicle.hpp"
#include "eCamera_mode.hpp"

#include <iostream>


namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - CW2";

	constexpr float kMovementPerSecond_ = 5.0f; // units per second
	constexpr float kMouseSensitivity_ = 0.01f; // radians per pixel
	constexpr Vec3f kLandpadPosition1_ = {5.f, 0.f, -5.f}; // Placed on the sea - y-axis is 0
	constexpr Vec3f kLandpadPosition2_ = {-2.1f, 0.f, 1.1f};
	constexpr Vec3f kLandpadColor_ = {1.f, 1.f, 1.f};
	constexpr Vec3f kBurgandy_ = {0.333f, 0.01f, 0.03f};
	constexpr Vec3f kGrayBurgandy_ = {0.232f, 0.03f, 0.045f};
	constexpr Vec3f kLightBlue_ = {0.745f, 0.851f, 0.867f}; // #bed9dd
	constexpr Vec3f kGray_ = {0.718f, 0.718f, 0.718f}; // #b7b7b7

	struct State_
	{
		ShaderProgram* progTexture;
		ShaderProgram* progColor;
		CameraMode camMode = FreeCamera;

		Vec3f trackCameraDistanceOffset = {0.f, -1.f, -5.f};
		Vec3f groundCameraPosition = {-2.f, -0.1f, 1.f};
		// Vec3f groundCameraPosition = kLandpadPosition2_;

		
		struct CamCtrl_
		{
			bool cameraActive;
			bool isForward, isBackward, isLeft, isRight, isUp, isDown;
			bool actionSpeedUp, actionSlowDown;
			
			float phi, theta;
			float movementSpeed = kMovementPerSecond_;
			
			Vec3f cameraPosition = Vec3f{0.0f, -0.2f, 0.0f}; // Starting position
			Vec3f cameraForwardDirection = Vec3f{0.0f, 0.0f, -1.0f}; // Initially facing along -Z
			Vec3f cameraUpDirection = Vec3f{0.0f, 1.0f, 0.0f}; // Y is up
			Vec3f cameraRightDirection = Vec3f{1.0f, 0.0f, 0.0f}; // X is right

			float lastX, lastY;
		} camControl;

		struct SpaceshipCtrl_
		{
			bool isAnimation = false, isAnimationPaused = true;

			float shipInitSpeed = 0.f, initAngle = 0.f, radius = 5.f, acceleration = 0.01f;
			float shipSpeed = shipInitSpeed;
			float angle = initAngle;
			
			Vec3f shipInitPosition = kLandpadPosition1_;
			Vec3f shipPosition = shipInitPosition;
			Vec3f shipDirection = {0.0f, 1.0f, 0.0f}; // up to Y axis direction

			Mat44f shipRotation = kIdentity44f;
		} spaceship;
	};

	void glfw_callback_error_( int, char const* );
	void glfw_callback_key_( GLFWwindow*, int, int, int, int );
	void glfw_callback_motion_( GLFWwindow*, double, double );
	void glfw_callback_mouse_button_( GLFWwindow*, int, int, int );
	void update_camera_position_by_cam_movement( State_::CamCtrl_&, float );
	void update_free_camera_direction_vectors( State_::CamCtrl_&);
	void update_ship_position( State_::SpaceshipCtrl_ &, float );
	void update_ship_direction_vectors( State_::SpaceshipCtrl_ & );	
	void update_camera ( State_&, float );



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
	ShaderProgram progColor( {{ GL_VERTEX_SHADER, "assets/cw2/default.vert" }, { GL_FRAGMENT_SHADER, "assets/cw2/colored_objects.frag" }} );
	
	state.progTexture = &progTexture;
	state.progColor = &progColor;

	// Load Meshes and Textures
	SimpleMeshData langersoMesh = load_wavefront_obj("assets/cw2/langerso.obj"); // Load Mesh
	GLuint langersoVAO = create_vao(langersoMesh); // Returns a VAO pointer from the Attributes object
	std::size_t langersoVertices = langersoMesh.positions.size() ; // Calculate the number of vertices to draw later
	GLuint textureID = load_texture_2d("assets/cw2/L3211E-4k.jpg");  // Load Texture

	SimpleMeshData landingpadMesh = load_wavefront_obj("assets/cw2/landingpad.obj"); // Load Mesh
	GLuint landingpadVAO = create_vao(landingpadMesh); // Returns a VAO pointer from the Attributes object
	std::size_t landingpadVertices = landingpadMesh.positions.size() ; // Calculate the number of vertices to draw later

	float cylinderBodyRadius = 0.07f; 
	float cylinderBoosterRadius = 0.03f;
	float cubeHeight = 0.08f;
	float cubeRadius = 0.01f;

	// Rocket base
	SimpleMeshData cylinderMesh1 = make_cylinder(
		true, 64, kGrayBurgandy_, 
		make_translation( { 0.f, 0.05f, 0.f }) * 
		make_scaling( cylinderBodyRadius, 1.f, cylinderBodyRadius ) * 
		make_rotation_z(std::numbers::pi_v<float> / 2.0f));	
	SimpleMeshData coneMesh1 = make_cone(
		true, 64, kGrayBurgandy_ , 
		make_translation( { 0.f, 1.05f, 0.f }) * 
		make_scaling( cylinderBodyRadius, 0.2f, cylinderBodyRadius ) * 
		make_rotation_z(std::numbers::pi_v<float> / 2.0f));

	// Boosters
	SimpleMeshData cylinderMesh2 = make_cylinder(
		true, 64, kBurgandy_, 
		make_translation( { cylinderBodyRadius * 1.05f, 0.03f, 0.f }) * 
		make_scaling( cylinderBoosterRadius, 0.5f, cylinderBoosterRadius ) * 
		make_rotation_z(std::numbers::pi_v<float> / 2.0f));	
	SimpleMeshData coneMesh2 = make_cone(
		true, 64, kBurgandy_ , 
		make_translation( { cylinderBodyRadius * 1.05f, 0.53f, 0.f }) * 
		make_scaling( cylinderBoosterRadius, 0.07f, cylinderBoosterRadius ) * 
		make_rotation_z(std::numbers::pi_v<float> / 2.0f));
	SimpleMeshData cylinderMesh3 = make_cylinder(
		true, 64, kBurgandy_, 
		make_translation( { -cylinderBodyRadius * 1.05f, 0.03f, 0.f }) * 
		make_scaling( cylinderBoosterRadius, 0.5f, cylinderBoosterRadius ) * 
		make_rotation_z(std::numbers::pi_v<float> / 2.0f));	
	SimpleMeshData coneMesh3 = make_cone(
		true, 64, kBurgandy_ , 
		make_translation( { -cylinderBodyRadius * 1.05f, 0.53f, 0.f }) * 
		make_scaling( cylinderBoosterRadius, 0.07f, cylinderBoosterRadius ) * 
		make_rotation_z(std::numbers::pi_v<float> / 2.0f));

	// Legs 
	SimpleMeshData cubeMesh1 = make_cube(
		kLightBlue_ , 
		make_translation( { 0.f, cubeHeight, cylinderBodyRadius }) * 
		make_scaling( cubeRadius, cubeHeight, cubeRadius ));
	SimpleMeshData cubeMesh2 = make_cube(
		kLightBlue_ , 
		make_translation( { cylinderBodyRadius * sqrtf(3.f) / 2.0f, cubeHeight, -cylinderBodyRadius / 2.0f }) *
		make_scaling( cubeRadius, cubeHeight, cubeRadius )); 
	SimpleMeshData cubeMesh3 = make_cube(
		kLightBlue_ , 
		make_translation( { -cylinderBodyRadius * sqrtf(3.f) / 2.0f, cubeHeight , -cylinderBodyRadius / 2.0f }) * 
		make_scaling( cubeRadius, cubeHeight, cubeRadius ));

	GLuint vehicleVAO = create_vao( concatenate( {cylinderMesh1,cylinderMesh2,cylinderMesh3, coneMesh1, coneMesh2, coneMesh3, cubeMesh1, cubeMesh2, cubeMesh3} ));
	std::size_t vehicleVertices = cylinderMesh1.positions.size() + cylinderMesh2.positions.size() + cylinderMesh2.positions.size() + 
		coneMesh1.positions.size() + coneMesh2.positions.size() +coneMesh3.positions.size() + cubeMesh1.positions.size() * 3;
	

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

		// Update camera state
		update_camera(state, dt);


		// Update spaceship state
		update_ship_position(state.spaceship, dt);
		// update_ship_direction_vectors(state.spaceship);

		// Define the camera rotation matrices
		Mat44f Rx = make_rotation_x(state.camControl.theta); // Theta controls vertical rotation -  yaw
		Mat44f Ry = make_rotation_y(state.camControl.phi); // Phi controls the horizontal rotation	- pitch	

		// Define the spaceship rotation matrix
		state.spaceship.shipRotation =  make_rotation_x(state.spaceship.angle);

		// transformation for langerso
		Mat44f T = make_translation({state.camControl.cameraPosition.x, state.camControl.cameraPosition.y, -state.camControl.cameraPosition.z}); // Define the camera position in world space
		Mat44f world2camera = Rx * Ry * T; // Create world to camera matrix by first translating and then rotating
		Mat44f projection = make_perspective_projection(  60.f * std::numbers::pi_v<float> / 180.f, fbwidth/float(fbheight), 0.1f, 100.0f );

		// Draw scene
		// Render langerso model
		render_model(progTexture, langersoVAO, projection, world2camera, {0.f, 0.f, 0.f}, textureID, langersoVertices );
	
		// Render 1st landingpad
		render_model(progColor, landingpadVAO, projection, world2camera, kLandpadPosition1_, landingpadVertices );
	
		// Render 2nd landingpad
		render_model(progColor, landingpadVAO, projection, world2camera, kLandpadPosition2_, landingpadVertices );

		// Render spaceship
		render_model(progColor, vehicleVAO, projection, world2camera , state.spaceship.shipPosition, vehicleVertices );
		
		// render_model(progColor, vehicleVAO, projection, world2camera , state.spaceship.shipPosition, state.spaceship.shipRotation, vehicleVertices );


		OGL_CHECKPOINT_DEBUG();

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		// glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		
		// Reset state
		glBindVertexArray( 0 );
		glUseProgram( 0 );	

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	state.progTexture = nullptr;
	state.progColor = nullptr;
	
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

			if ( GLFW_KEY_F == aKey && GLFW_PRESS == aAction )
			{
				state->spaceship.isAnimation = !state->spaceship.isAnimation; 
				state->spaceship.isAnimationPaused = !state->spaceship.isAnimationPaused;
			}
			if ( GLFW_KEY_R == aKey && GLFW_PRESS == aAction )
			{
				state->spaceship.isAnimation = false;
				state->spaceship.isAnimationPaused = true;
				state->spaceship.shipPosition = state->spaceship.shipInitPosition;
				state->spaceship.shipSpeed = state->spaceship.shipInitSpeed; 
				state->spaceship.angle = state->spaceship.initAngle; 
				state->spaceship.shipDirection = {0.0f, 1.0f, 0.0f}; 
			}
			if ( GLFW_KEY_C == aKey && GLFW_PRESS == aAction ) 
			{
            	state->camMode = static_cast<CameraMode>((state->camMode + 1) % 3);
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
	void update_camera_position_by_cam_movement( State_::CamCtrl_ &aCamControl, float dt)
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

	}

	void update_free_camera_direction_vectors( State_::CamCtrl_ &aCamControl)
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

	void update_camera (State_& state, float dt)
	{
		Vec3f directionToVehicle;

		switch (state.camMode) 
		{
        	case FreeCamera:				
				update_camera_position_by_cam_movement(state.camControl, dt);
				update_free_camera_direction_vectors(state.camControl);
				break;
			case FixedTrackCamera:
				state.camControl.phi = 0.f;
				state.camControl.theta = 0.f;
				directionToVehicle = normalize(state.spaceship.shipPosition - state.camControl.cameraPosition);
				state.camControl.cameraPosition = state.trackCameraDistanceOffset - state.spaceship.shipPosition;
				break;
			case GroundCamera:
				state.camControl.cameraPosition = state.groundCameraPosition;
				directionToVehicle = normalize(state.spaceship.shipPosition - state.camControl.cameraPosition);
				state.camControl.phi = std::atan2(directionToVehicle.x, -directionToVehicle.z);
				state.camControl.theta = - std::atan2(directionToVehicle.y, std::sqrt(directionToVehicle.x * directionToVehicle.x + directionToVehicle.z * directionToVehicle.z));
				break;
		}
	}

	void update_ship_position( State_::SpaceshipCtrl_ &aShipControl, float dt)
	{
		
		if ( !aShipControl.isAnimation || aShipControl.isAnimationPaused ) return; // No movement if the animation is inactive

		// Adjust ship movement speed
        aShipControl.shipSpeed += aShipControl.acceleration * dt;
        aShipControl.shipSpeed = std::min(aShipControl.shipSpeed, 5.f); // Prevent speed from going too fast

		// Adjust ship direction of movement
		aShipControl.angle += dt * aShipControl.shipSpeed / aShipControl.radius;
        aShipControl.angle = std::min(aShipControl.angle, static_cast<float>(M_PI) - 0.1f); 
		// std::fprintf( stderr, "angle (%f)\n", aShipControl.angle);


		// Define the curved trajectory
		float heightFactor = 2.0;  // Adjust this factor to control the vertical lift speed
		aShipControl.shipPosition.x += aShipControl.radius * sin(aShipControl.angle);  // Sine for horizontal motion
		aShipControl.shipPosition.y += dt * heightFactor ;  // Linear ascent
		// aShipControl.shipRotation = make_rotation_x((dt * heightFactor) / (aShipControl.radius * sin(aShipControl.angle)));
		// aShipControl.shipDirection = normalize(Vec3f(cos(aShipControl.angle), 0, -sin(aShipControl.angle)));
		
		// Adjust ship position
		// aShipControl.shipPosition += aShipControl.shipDirection * aShipControl.shipSpeed * dt;
		// std::fprintf( stderr, "position (%f,%f,%f)\n", aShipControl.shipPosition.x, aShipControl.shipPosition.y, aShipControl.shipPosition.z);
		// std::fprintf( stderr, "direction (%f,%f,%f)\n", aShipControl.shipDirection.x, aShipControl.shipDirection.y, aShipControl.shipDirection.z);
	}

// 	void update_ship_direction_vectors( State_::SpaceshipCtrl_ &aShipControl )
// 	{
		
// 		aShipControl.shipDirection = normalize(Vec3f(
// 			cos(shipControl.angle),
// 			 0.0f, sin(shipControl.angle)));
// 	}
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
