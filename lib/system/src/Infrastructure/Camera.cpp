#include <helsinki/System/Infrastructure/Camera.hpp>

namespace hl
{
    Camera::Camera(
        glm::vec3 position,
        glm::vec3 up,
        float yaw,
        float pitch
    )
    {
        this->position = position;
        this->up = up;
        this->worldUp = { 0.0f, 1.0f, 0.0f };
        this->yaw = yaw;
        this->pitch = pitch;
        this->zoom = 45.0f;
        updateCameraVectors();
    }
    void Camera::processKeyboard(CameraMovement direction, float deltaTime)
    {
        float velocity = movementSpeed * deltaTime;

        if (direction == CameraMovement::FORWARD)
        {
            position += front * velocity;
        }
        if (direction == CameraMovement::BACKWARD)
        {
            position -= front * velocity;
        }
        if (direction == CameraMovement::LEFT)
        {
            position -= right * velocity;
        }
        if (direction == CameraMovement::RIGHT)
        {
            position += right * velocity;
        }
        if (direction == CameraMovement::UP)
        {
            position += up * velocity;
        }
        if (direction == CameraMovement::DOWN)
        {
            position -= up * velocity;
        }
    }

    void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch)
    {
        xOffset *= mouseSensitivity;
        yOffset *= mouseSensitivity;

        yaw += xOffset;
        pitch += yOffset;

        // Constrain pitch to avoid flipping
        if (constrainPitch)
        {
            if (pitch > 89.0f)
            {
                pitch = 89.0f;
            }
            if (pitch < -89.0f)
            {
                pitch = -89.0f;
            }
        }

        // Update camera vectors based on updated Euler angles
        updateCameraVectors();
    }

    void Camera::updateCameraVectors()
    {
        // Calculate the new front vector
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        // Recalculate the right and up vectors
        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }

    glm::mat4 Camera::getViewMatrix() const
    {
        return glm::lookAt(position, position + front, up);
    }

    glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const
    {
        return glm::perspective(glm::radians(zoom), aspectRatio, nearPlane, farPlane);
    }

}