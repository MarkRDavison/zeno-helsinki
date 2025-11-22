#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp> 

namespace hl
{
	enum class CameraMovement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	class Camera
	{
	public:
		// Constructor with sensible defaults for common use cases
		// Provides flexibility while ensuring the camera starts in a predictable state
		Camera(
			glm::vec3 position,
			glm::vec3 up,
			float yaw,
			float pitch
		);

		// Matrix generation for graphics pipeline integration
		// These methods bridge between the camera's spatial representation and GPU requirements
		glm::mat4 getViewMatrix() const;
		glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;
		
		// Input processing methods for different interaction modalities
		// Each method handles a specific type of user input with appropriate transformations
		void processKeyboard(CameraMovement direction, float deltaTime);     // Keyboard-based translation
		void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);  // Mouse-based rotation
		void processMouseScroll(float yOffset);                              // Scroll-based zoom control

		// Property access methods for external systems
		// Provide controlled access to internal state without exposing implementation details
		glm::vec3 getPosition() const { return position; }
		glm::vec3 getFront() const { return front; }
		float getZoom() const { return zoom; }
		void setZoom(float newZoom) { zoom = newZoom; }
	private:
		// Internal coordinate system maintenance
		// Ensures mathematical consistency when orientation changes occur
		void updateCameraVectors();

	protected:
		// Spatial positioning and orientation vectors
		// These form the camera's local coordinate system in world space
		glm::vec3 position;     // Camera's location in world coordinates
		glm::vec3 front;        // Forward direction (where camera is looking)
		glm::vec3 up;           // Camera's local up direction (for roll control)
		glm::vec3 right;        // Camera's local right direction (perpendicular to front and up)
		glm::vec3 worldUp;      // Global up vector reference (typically Y-axis)

		// Rotation representation using Euler angles
		// Provides intuitive control while managing gimbal lock and other rotation complexities
		float yaw;              // Horizontal rotation around the world up-axis (left-right looking)
		float pitch;            // Vertical rotation around the camera's right axis (up-down looking)

		// User interaction and behavior parameters
		// These control how the camera responds to input and environmental factors
		float movementSpeed{ 20.0f };    // Units per second for translation movement
		float mouseSensitivity{ 1.0f }; // Multiplier for mouse input to rotation angle conversion
		float zoom;             // Field of view control for perspective projection
	};

}