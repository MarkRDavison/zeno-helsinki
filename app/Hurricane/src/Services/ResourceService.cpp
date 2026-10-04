#include <Services/ResourceService.hpp>

namespace hur
{

	void ResourceService::addSpriteIndexAndSize(const std::string& name, std::size_t index, glm::vec2 size, glm::vec4 uvRect)
	{
		_sprites.insert({ name, { index, size, uvRect } });
	}

	std::size_t ResourceService::getIndex(const std::string& name) const
	{
		return _sprites.at(name).index;
	}

	glm::vec2 ResourceService::getSize(const std::string& name) const
	{
		return _sprites.at(name).size;
	}

	glm::vec4 ResourceService::getUvRect(const std::string& name) const
	{
		return _sprites.at(name).uvRect;
	}
}
