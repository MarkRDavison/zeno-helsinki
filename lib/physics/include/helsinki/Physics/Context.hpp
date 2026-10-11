#pragma once

#include <helsinki/Physics/Export.hpp>

#include <helsinki/System/Utils/NonCopyable.hpp>
#include <memory>

namespace hl::physics
{
	class HELSINKI_PHYSICS_EXPORT Context : public hl::NonCopyable
	{
	public:
		Context();
		~Context() override;

		bool init();
		void shutdown();
		bool ok() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}
