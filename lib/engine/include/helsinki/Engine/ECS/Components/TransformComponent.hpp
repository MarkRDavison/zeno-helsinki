#pragma once

#include <helsinki/Engine/ECS/Component.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace hl
{
	class TransformComponent : public Component
    {
    private:
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identity quaternion
        glm::vec3 scale = glm::vec3(1.0f);

        // Cached transformation matrix
        mutable glm::mat4 transformMatrix = glm::mat4(1.0f);
        mutable bool transformDirty = true;

    public:
        void SetPosition(const glm::vec3& pos);

        void SetRotation(const glm::quat& rot);

        void SetScale(const glm::vec3& s);

        const glm::vec3& GetPosition() const { return position; }
        const glm::quat& GetRotation() const { return rotation; }
        const glm::vec3& GetScale() const { return scale; }

        glm::mat4 GetTransformMatrix() const;
    };
}