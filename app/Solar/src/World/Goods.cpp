#include <World/Goods.hpp>

namespace slr
{

	const char* goodName(long long goodId)
	{
		if (goodId == SteelGoodId)
		{
			return "steel";
		}

		return "unknown";
	}

}
