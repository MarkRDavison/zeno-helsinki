#include <Candidates/UiRect.hpp>
#include <Candidates/UiRoot.hpp>

namespace hl
{

	UiRect::UiRect(
		const UiRoot& root, 
		const UiRect* parent
	) :
		_root(root),
		_parent(parent)
	{

	}

}