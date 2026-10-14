#pragma once

#include <Views/MainView.hpp>
#include <World/WorldData.hpp>

namespace slr
{

	class SystemView
	{
	public:
		void draw(const WorldData& world, MainView& mainView) const;
	};

}
