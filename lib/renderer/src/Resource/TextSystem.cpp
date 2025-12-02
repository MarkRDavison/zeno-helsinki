#include <helsinki/Renderer/Resource/TextSystem.hpp>

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

	int TextSystem::registerText(const std::string& text, const std::string& font)
	{
		return registerText(-1, text, font);
	}
	int TextSystem::registerText(int id, const std::string& text, const std::string& font)
	{
		if (id >= 0)
		{
			auto t= _textInfo.at(id);
			t->_vertexBuffer.destroy();
			delete t;
			_textInfo.erase(id);
		}

		id = _nextId++;

		_textInfo.insert({ id, new Text(_device) });

		generateText(id, text, font);

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

	void TextSystem::generateText(int id, const std::string& text, const std::string& font)
	{
		auto t = _textInfo.at(id);

		auto fontResource = _resourceManager.GetResource<hl::FontResource>(font);
		if (fontResource == nullptr)
		{
			throw std::runtime_error("INVALID FONT!");
		}

		const auto& vert = fontResource->generateTextVertexes(text);

		t->_vertexCount = (uint32_t)vert.size();

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