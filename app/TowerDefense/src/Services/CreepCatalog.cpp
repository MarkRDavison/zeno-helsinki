#include <Services/CreepCatalog.hpp>
#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <stdexcept>

namespace tower
{
	namespace
	{
		[[noreturn]] void fail(const std::string& message)
		{
			throw std::runtime_error(message);
		}

		const hl::JsonNode& field(const hl::JsonNode& row, const char* name)
		{
			try
			{
				return row[name];
			}
			catch (const std::string& error)
			{
				fail(error);
			}
		}

		std::string requireString(const hl::JsonNode& row, const char* name)
		{
			const auto& node = field(row, name);
			if (node.type != hl::JsonNode::Type::ValueString || node.content.empty())
			{
				fail(std::string("creeps.json: '") + name + "' must be a non-empty string");
			}

			return node.content;
		}

		int requireHealth(const hl::JsonNode& row)
		{
			const auto& node = field(row, "health");
			if (node.type != hl::JsonNode::Type::ValueInteger || node.integer < 1)
			{
				fail("creeps.json: 'health' must be an integer >= 1");
			}

			return node.integer;
		}

		float requireSpeed(const hl::JsonNode& row)
		{
			const auto& node = field(row, "speed");
			if (node.type == hl::JsonNode::Type::ValueNumber)
			{
				if (node.number <= 0.0f)
				{
					fail("creeps.json: 'speed' must be > 0");
				}

				return node.number;
			}

			if (node.type == hl::JsonNode::Type::ValueInteger)
			{
				if (node.integer <= 0)
				{
					fail("creeps.json: 'speed' must be > 0");
				}

				return static_cast<float>(node.integer);
			}

			fail("creeps.json: 'speed' must be a number");
		}
	}

	void CreepCatalog::load(const std::string& path)
	{
		_defs.clear();
		_byId.clear();

		const auto doc = hl::Json::parseFromText(hl::String::readFile(path));
		if (doc.m_Root == nullptr || doc.m_Root->type != hl::JsonNode::Type::Array)
		{
			fail("creeps.json: root must be an array");
		}

		for (const auto* row : doc.m_Root->children)
		{
			if (row == nullptr || row->type != hl::JsonNode::Type::Object)
			{
				fail("creeps.json: each entry must be an object");
			}

			CreepDef def;
			def.id = requireString(*row, "id");
			def.model = requireString(*row, "model");
			def.health = requireHealth(*row);
			def.speed = requireSpeed(*row);
			if (_byId.contains(def.id))
			{
				fail("creeps.json: duplicate id '" + def.id + "'");
			}

			_byId.emplace(def.id, _defs.size());
			_defs.push_back(std::move(def));
		}

		if (_defs.empty())
		{
			fail("creeps.json: catalog is empty");
		}

		if (find("runner") == nullptr || find("tank") == nullptr)
		{
			fail("creeps.json: must define id 'runner' and 'tank'");
		}
	}

	const CreepDef* CreepCatalog::find(std::string_view id) const
	{
		const auto it = _byId.find(std::string(id));
		if (it == _byId.end())
		{
			return nullptr;
		}

		return &_defs[it->second];
	}

	const std::vector<CreepDef>& CreepCatalog::all() const
	{
		return _defs;
	}
}
