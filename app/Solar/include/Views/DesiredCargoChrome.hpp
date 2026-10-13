#pragma once

#include <World/DesiredCargo.hpp>

namespace slr
{

	class DesiredCargoChrome
	{
	public:
		bool draw(DesiredCargo& draft, GoodAmount used, GoodAmount capacity);
	};

}
