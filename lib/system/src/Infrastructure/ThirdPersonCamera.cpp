#include <helsinki/System/Infrastructure/ThirdPersonCamera.hpp>

namespace hl
{

    void ThirdPersonCamera::updatePosition(
        const glm::vec3& targetPos,
        const glm::vec3& targetFwd,
        float deltaTime
    )
    {
        // Update target properties
        targetPosition = targetPos;
        targetForward = glm::normalize(targetFwd);

        // Calculate the desired camera position
        // Position the camera behind and above the character
        glm::vec3 offset = -targetForward * followDistance;
        offset.y = followHeight;

        desiredPosition = targetPosition + offset;

        // Smooth camera movement using exponential smoothing
        position = glm::mix(position, desiredPosition, 1.0f - pow(followSmoothness, deltaTime * 60.0f));

        // Update the camera to look at the target
        front = glm::normalize(targetPosition - position);

        // Recalculate right and up vectors
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

    void ThirdPersonCamera::handleOcclusion(const void* /*scene*/)
    {
        /*
        // Cast a ray from the target to the desired camera position
        Ray ray;
        ray.origin = targetPosition;
        ray.direction = glm::normalize(desiredPosition - targetPosition);

        // Check for intersections with scene objects
        RaycastHit hit;
        if (scene.raycast(ray, hit, glm::length(desiredPosition - targetPosition)))
        {
            // If there's an intersection, move the camera to the hit point
            // minus a small offset to avoid clipping
            float offsetDistance = 0.2f;
            position = hit.point - (ray.direction * offsetDistance);

            // Ensure we don't get too close to the target
            float currentDistance = glm::length(position - targetPosition);
            if (currentDistance < minDistance)
            {
                position = targetPosition + ray.direction * minDistance;
            }

            // Update the camera to look at the target
            front = glm::normalize(targetPosition - position);
            right = glm::normalize(glm::cross(front, worldUp));
            up = glm::normalize(glm::cross(right, front));
        }
        */
    }

    void ThirdPersonCamera::orbit(float horizontalAngle, float verticalAngle)
    {
        // Update yaw and pitch based on input
        yaw += horizontalAngle;
        pitch += verticalAngle;

        // Constrain pitch to avoid flipping
        if (pitch > 89.0f)
        {
            pitch = 89.0f;
        }
        if (pitch < -89.0f)
        {
            pitch = -89.0f;
        }

        // Calculate the new camera position based on spherical coordinates
        float radius = followDistance;
        float yawRad = glm::radians(yaw);
        float pitchRad = glm::radians(pitch);

        // Convert spherical coordinates to Cartesian
        glm::vec3 offset;
        offset.x = radius * cos(yawRad) * cos(pitchRad);
        offset.y = radius * sin(pitchRad);
        offset.z = radius * sin(yawRad) * cos(pitchRad);

        // Set the desired position
        desiredPosition = targetPosition + offset;

        // Update camera vectors
        front = glm::normalize(targetPosition - desiredPosition);
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

}