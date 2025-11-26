#include <helsinki/Engine/ECS/Entity.hpp>
#include <helsinki/System/Utils/String.hpp>

namespace hl
{

	void Entity::AddTag(const std::string& tag)
	{
		tags.insert(String::fnv1a_32(tag));
	}
	void Entity::ClearTag(const std::string& tag)
	{
		tags.erase(String::fnv1a_32(tag));
	}
	bool Entity::HasTag(const std::string& tag) const
	{
		return tags.count(String::fnv1a_32(tag)) > 0;
	}

}