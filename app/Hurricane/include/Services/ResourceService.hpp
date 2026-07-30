#pragma once

#include <string>
#include <unordered_map>
#include <helsinki/System/glm.hpp>

namespace hur
{

	class ResourceService
	{
	public:
		void addSpriteIndexAndSize(const std::string& name, std::size_t index, glm::vec2 size);
		std::size_t getIndex(const std::string& name) const;
		glm::vec2 getSize(const std::string& name) const;

	private:
		std::unordered_map<std::string, std::pair<std::size_t, glm::vec2>> _spriteToIndexAndSize;

	};

}