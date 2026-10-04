#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/System/Resource/LogicalResource.hpp>

namespace hl
{

	TextSystem::TextSystem(
		VulkanDevice& device,
		VulkanCommandPool& transferPool,
		ResourceManager& resourceManager
	) :
		_device(device),
		_transferPool(transferPool),
		_resourceManager(resourceManager)
	{

	}

	int TextSystem::registerText(const std::string& text, const std::string& font, unsigned size)
	{
		return registerText(-1, text, font, size);
	}
	int TextSystem::registerText(int id, const std::string& text, const std::string& font, unsigned size)
	{
		if (id >= 0)
		{
			_textToDestroy.push(id);
		}

		// TODO: Need to handle re-using ids
		// TODO: Some system to keep a backlog of text's and re-use them? Commonly used etc...

		id = _nextId++;

		_textInfo.insert({ id, new Text(_device) });

		generateText(id, text, font, size);

		return id;
	}

	void TextSystem::destroy()
	{
		for (auto& [id, text] : _textInfo)
		{
			text->_vertexBuffer.destroy();
			delete text;
		}

		_textInfo.clear();
	}

	const Text& TextSystem::getText(int id) const
	{
		// TODO: fallback???
		return *_textInfo.at(id);
	}

	// TODO: NEEDS CACHE
	glm::vec4 TextSystem::getTextSize(int id) const
	{
		const auto& t = getText(id);

		auto fontResource = _resourceManager.GetResource<hl::FontResource>(t._font);

		if (fontResource == nullptr)
		{
			throw std::runtime_error("INVALID FONT!");
		}
		// TODO: generateTextVertexes should return verticies and bounds?
		const auto& vert = fontResource->generateTextVertexes(t._text, t._size);

		glm::vec2 min{};
		glm::vec2 max{};

		for (const auto& vertex : vert)
		{
			min.x = std::min(min.x, vertex.pos.x);
			min.y = std::min(min.y, vertex.pos.y);

			max.x = std::max(max.x, vertex.pos.x);
			max.y = std::max(max.y, vertex.pos.y);
		}

		return glm::vec4(min.x, min.y, max.x - min.x, max.y - min.y);
	}

	uint32_t TextSystem::getFontAtlasIndex(FontType fontType, const std::string& fontId) const
	{
		const char* sheetName = fontType == FontType::Rasterised
			? RasterAtlasName
			: SdfAtlasName;

		const auto* logical = _resourceManager.GetResource<LogicalResource>(sheetName);
		if (logical == nullptr)
		{
			throw std::runtime_error(std::string("Missing font atlas sheet '") + sheetName + "'");
		}

		return fontAtlasIndex(logical->GetChildren(), fontId);
	}

	void TextSystem::processDeferredTextDestruction(int count /*= -1*/)
	{
		if (count < 0)
		{
			count = 100;
		}

		while (count > 0 && _textToDestroy.size() > 0)
		{
			const auto idToDestroy = _textToDestroy.front();
			_textToDestroy.pop();
			auto t = _textInfo.at(idToDestroy);
			t->_vertexBuffer.destroy();
			delete t;
			_textInfo.erase(idToDestroy);
			count--;
		}
	}

	void TextSystem::generateText(int id, const std::string& text, const std::string& font, unsigned size)
	{
		auto t = _textInfo.at(id);

		auto fontResource = _resourceManager.GetResource<hl::FontResource>(font);

		if (fontResource == nullptr)
		{
			throw std::runtime_error("INVALID FONT!");
		}

		const auto& vert = fontResource->generateTextVertexes(text, size);

		t->_fontType = fontResource->getFontType();
		t->_vertexCount = (uint32_t)vert.size();
		t->_size = size;
		t->_text = text;
		t->_font = font;

		VkDeviceSize bufferSize = sizeof(vert[0]) * vert.size();

		hl::VulkanBuffer stagingBuffer(_device);
		stagingBuffer.create(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.mapMemory(
			vert.data());

		t->_vertexBuffer.create(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		stagingBuffer.copyToBuffer(
			_transferPool,
			bufferSize,
			t->_vertexBuffer);

		stagingBuffer.destroy();
	}
}