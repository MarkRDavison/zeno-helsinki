#pragma once

#include <helsinki/System/Infrastructure/Camera.hpp>

namespace hl
{

    class ThirdPersonCamera : public Camera
    {
    public:
        // Constructor with gameplay-tuned defaults
        // Default values chosen for common third-person game scenarios
        ThirdPersonCamera(
            float followDistance = 5.0f,        // Medium distance providing good character visibility and environment context
            float followHeight = 2.0f,          // Height above target for clear sightlines over low obstacles
            float followSmoothness = 0.1f,      // Moderate smoothing for responsive but stable camera motion
            float minDistance = 1.0f            // Minimum distance to prevent uncomfortable close-ups
        );

        // Core functionality methods for camera behavior control
        void updatePosition(const glm::vec3& targetPos, const glm::vec3& targetFwd, float deltaTime);
        void handleOcclusion(const void* scene);
        void orbit(float horizontalAngle, float verticalAngle);

        // Runtime configuration methods for dynamic camera adjustment
        void setFollowDistance(float distance) { followDistance = distance; }
        void setFollowHeight(float height) { followHeight = height; }
        void setFollowSmoothness(float smoothness) { followSmoothness = smoothness; }

    private:
        // Target entity tracking and spatial relationship data
        // These properties define the relationship between camera and the character being followed
        glm::vec3 targetPosition;      // Current world position of the target character
        glm::vec3 targetForward;       // Target's forward direction vector for contextual camera positioning
        // Camera behavior configuration parameters
        // These values control the aesthetic and functional characteristics of camera following
        float followDistance;          // Desired distance from target (affects intimacy and field of view)
        float followHeight;            // Height offset above target (provides better scene visibility)
        float followSmoothness;        // Interpolation factor for smooth camera transitions (0 = instant, 1 = never)
        // Occlusion avoidance and collision management
        // These parameters control how the camera responds to environmental obstacles
        float minDistance;             // Minimum allowed distance from target (prevents camera from getting too close)
        float raycastDistance;         // Maximum distance for occlusion detection rays
        // Internal computational state for smooth motion control
        // These variables manage the mathematical aspects of camera positioning and movement
        glm::vec3 desiredPosition;     // Target position the camera wants to reach (before collision adjustments)
        glm::vec3 smoothDampVelocity;  // Velocity state for smooth damping interpolation algorithms

    };

}