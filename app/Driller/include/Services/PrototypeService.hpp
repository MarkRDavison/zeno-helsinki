#pragma once

#include <helsinki/System/Utils/String.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace drl
{

	inline long long prototypeIdFromName(std::string_view name)
	{
		return static_cast<long long>(hl::String::fnv1a_32(name));
	}

	template<typename TInstance, typename TPrototype>
	class IPrototypeService
	{
	public:
		virtual ~IPrototypeService() = default;

		virtual void registerPrototype(TPrototype prototype) = 0;
		virtual bool isPrototypeRegistered(long long prototypeId) const = 0;
		virtual long long getPrototypeId(const std::string& name) const = 0;
		virtual const TPrototype& getPrototype(long long prototypeId) const = 0;
		virtual TInstance createInstance(long long prototypeId) = 0;
	};

	template<typename TInstance, typename TPrototype>
	class PrototypeService : public IPrototypeService<TInstance, TPrototype>
	{
	public:
		~PrototypeService() override = default;

		void registerPrototype(TPrototype prototype) override
		{
			const long long id = prototypeIdFromName(prototype.name);
			_prototypes.insert_or_assign(id, std::move(prototype));
		}

		bool isPrototypeRegistered(long long prototypeId) const override
		{
			return _prototypes.contains(prototypeId);
		}

		long long getPrototypeId(const std::string& name) const override
		{
			return prototypeIdFromName(name);
		}

		const TPrototype& getPrototype(long long prototypeId) const override
		{
			const auto it = _prototypes.find(prototypeId);
			if (it == _prototypes.end())
			{
				throw std::runtime_error("Unknown prototype");
			}
			return it->second;
		}

		TInstance createInstance(long long prototypeId) override
		{
			return createInstanceFromPrototype(getPrototype(prototypeId));
		}

	protected:
		virtual TInstance createInstanceFromPrototype(const TPrototype& prototype) = 0;

		long long allocateInstanceId()
		{
			return _nextInstanceId++;
		}

	private:
		std::unordered_map<long long, TPrototype> _prototypes;
		long long _nextInstanceId{ 1 };
	};

}
