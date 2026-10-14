#pragma once

#include <optional>

namespace slr
{

	enum class MainViewKind
	{
		Galaxy,
		System
	};

	class MainView
	{
	public:
		MainViewKind kind() const
		{
			return _kind;
		}

		std::optional<long long> openedSystemId() const
		{
			return _openedSystemId;
		}

		bool isGalaxy() const
		{
			return _kind == MainViewKind::Galaxy;
		}

		bool isSystem() const
		{
			return _kind == MainViewKind::System;
		}

		void openSystem(long long systemId)
		{
			_kind = MainViewKind::System;
			_openedSystemId = systemId;
		}

		void returnToGalaxy()
		{
			_kind = MainViewKind::Galaxy;
			_openedSystemId.reset();
		}

	private:
		MainViewKind _kind{ MainViewKind::Galaxy };
		std::optional<long long> _openedSystemId;
	};

}
