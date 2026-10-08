#include <helsinki/Physics/Context.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/RegisterTypes.h>

#include <cstdarg>
#include <cstdio>
#include <stdexcept>

namespace hl::physics
{
	namespace
	{
		bool g_typesRegistered = false;

		void traceCallback(const char* format, ...)
		{
			va_list args;
			va_start(args, format);
			char buffer[1024];
			vsnprintf(buffer, sizeof(buffer), format, args);
			va_end(args);
			(void)buffer;
		}

#ifdef JPH_ENABLE_ASSERTS
		bool assertFailedCallback(
			const char* expression,
			const char* message,
			const char* file,
			JPH::uint line)
		{
			(void)expression;
			(void)message;
			(void)file;
			(void)line;
			return false;
		}
#endif
	}

	struct Context::Impl
	{
		bool initialized = false;
	};

	Context::Context()
		: _impl(std::make_unique<Impl>())
	{
	}

	Context::~Context()
	{
		shutdown();
	}

	bool Context::init()
	{
		if (_impl->initialized)
		{
			throw std::runtime_error("hl::physics::Context::init called twice without shutdown");
		}

		if (g_typesRegistered)
		{
			throw std::runtime_error("hl::physics::Context is already initialized in this process");
		}

		JPH::RegisterDefaultAllocator();
		JPH::Trace = traceCallback;
		JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assertFailedCallback;)

		JPH::Factory::sInstance = new JPH::Factory();
		JPH::RegisterTypes();

		g_typesRegistered = true;
		_impl->initialized = true;
		return true;
	}

	void Context::shutdown()
	{
		if (!_impl || !_impl->initialized)
		{
			return;
		}

		JPH::UnregisterTypes();
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

		g_typesRegistered = false;
		_impl->initialized = false;
	}

	bool Context::ok() const
	{
		return _impl && _impl->initialized;
	}
}
