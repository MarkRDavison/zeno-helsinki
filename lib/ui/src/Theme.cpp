#include <helsinki/Ui/Theme.hpp>

namespace hl::ui
{
	Theme& theme()
	{
		static Theme instance;
		return instance;
	}
}
