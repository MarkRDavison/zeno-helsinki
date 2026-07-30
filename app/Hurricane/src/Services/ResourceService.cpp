#include <Services/ResourceService.hpp>

namespace hur
{

	void ResourceService::addSpriteIndexAndSize(const std::string& name, std::size_t index, glm::vec2 size)
	{
		_spriteToIndexAndSize.insert({ name, { index, size } });
	}

	std::size_t ResourceService::getIndex(const std::string& name) const
	{
		return _spriteToIndexAndSize.at(name).first;
	}

	glm::vec2 ResourceService::getSize(const std::string& name) const
	{
		return _spriteToIndexAndSize.at(name).second;
	}
}