#include <Systems/RotateSystem.hpp>
#include <SceneCatalog.hpp>
#include <helsinki/Engine/ECS/Components/ModelComponent.hpp>
#include <helsinki/Engine/ECS/Components/TransformComponent.hpp>

namespace sk
{
	RotateSystem::RotateSystem(hl::Scene& scene) :
		_scene(scene)
	{
	}

	void RotateSystem::update(float delta)
	{
		_angle += 45.0f * delta;

		for (auto* entity : _scene.getEntitiesWithComponents<hl::TransformComponent, hl::ModelComponent>(RotateTag))
		{
			entity->GetComponent<hl::TransformComponent>()->SetRotation(glm::vec3(0.0f, _angle, 0.0f));
		}
	}
}
