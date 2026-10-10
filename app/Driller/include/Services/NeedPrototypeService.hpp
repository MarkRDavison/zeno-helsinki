#pragma once

#include <Entities/Need.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace drl
{

	class INeedPrototypeService
	{
	public:
		virtual ~INeedPrototypeService() = 0;

		virtual void registerPrototype(NeedPrototype prototype) = 0;
		virtual bool isPrototypeRegistered(NeedId prototypeId) const = 0;
		virtual NeedId getPrototypeId(const std::string& name) const = 0;
		virtual const NeedPrototype& getPrototype(NeedId prototypeId) const = 0;
		virtual const std::vector<NeedId>& registeredIds() const = 0;
	};

	inline INeedPrototypeService::~INeedPrototypeService() = default;

	class NeedPrototypeService : public INeedPrototypeService
	{
	public:
		~NeedPrototypeService() override = default;

		void registerPrototype(NeedPrototype prototype) override
		{
			const NeedId id = needIdFromName(prototype.name);
			if (!_prototypes.contains(id))
			{
				_ids.push_back(id);
			}
			_prototypes.insert_or_assign(id, std::move(prototype));
		}

		bool isPrototypeRegistered(NeedId prototypeId) const override
		{
			return _prototypes.contains(prototypeId);
		}

		NeedId getPrototypeId(const std::string& name) const override
		{
			return needIdFromName(name);
		}

		const NeedPrototype& getPrototype(NeedId prototypeId) const override
		{
			const auto it = _prototypes.find(prototypeId);
			if (it == _prototypes.end())
			{
				throw std::runtime_error("Unknown prototype");
			}
			return it->second;
		}

		const std::vector<NeedId>& registeredIds() const override
		{
			return _ids;
		}

	private:
		std::unordered_map<NeedId, NeedPrototype> _prototypes;
		std::vector<NeedId> _ids;
	};

}
