#pragma once

#include <helsinki/Engine/ECS/System.hpp>
#include <helsinki/Engine/Scene/Scene.hpp>
#include <functional>

namespace tower
{
	class StatusCatalog;

	class StatusSystem : public hl::System
	{
	public:
		StatusSystem(hl::Scene& scene, const StatusCatalog& statuses);
		void update(float delta) override;

		std::function<void()> onKill;

	private:
		hl::Scene& _scene;
		const StatusCatalog& _statuses;
	};
}
