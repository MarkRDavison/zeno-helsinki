#pragma once

namespace hl
{
	class UiRoot;

	class UiRect
	{
	public:
		UiRect(const UiRoot& root, const UiRect *parent);

		float x{ 0.0f };
		float y{ 0.0f };
		float w{ 0.0f };
		float h{ 0.0f };

		float r{ 1.0f };
		float g{ 1.0f };
		float b{ 1.0f };

	protected:
		const UiRoot& _root;
		const UiRect* _parent;
	};

}