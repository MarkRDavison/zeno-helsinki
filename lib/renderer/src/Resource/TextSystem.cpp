#include <helsinki/Renderer/Resource/TextSystem.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>

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
			auto t= _textInfo.at(id);
			t->_vertexBuffer.destroy();
			delete t;
			_textInfo.erase(id);
		}

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

	std::unordered_map<std::string, uint32_t> TextSystem::bindFontsDescriptor(
		FontType fontType,
		VkDescriptorSet descriptorSet) const
	{
		std::unordered_map<std::string, uint32_t> fontToTextureIndexMap;

		const auto& allFonts = _resourceManager.GetAllResources<hl::FontResource>();

		std::vector<VkDescriptorImageInfo> imageInfos;

		for (const auto& f : allFonts)
		{
			if (f->getFontType() != fontType)
			{
				continue;
			}

			auto info = _resourceManager
				.GetResource<ImageSamplerResource>(
					f->GetId())
				->getDescriptorInfo(0);

			imageInfos.push_back(
				VkDescriptorImageInfo
				{
					.sampler = info.first,
					.imageView = info.second,
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				});

			fontToTextureIndexMap.insert({ f->GetId(), static_cast<uint32_t>(fontToTextureIndexMap.size())});
		}

		auto fallback = _resourceManager
			.GetResource<ImageSamplerResource>(
				"white") // TODO: CONSTANT: DEFAULT/FALLBACK TEXTURE
			->getDescriptorInfo(0);
		// TODO: Contant: MAX_FONTS
		while (imageInfos.size() < 64)
		{
			imageInfos.push_back(
				VkDescriptorImageInfo
				{
					.sampler = fallback.first,
					.imageView = fallback.second,
					.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				});
		}

		auto descriptorWrite = VkWriteDescriptorSet
		{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.dstSet = descriptorSet,
			.dstBinding = 1,
			.dstArrayElement = 0,
			.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			.pImageInfo = imageInfos.data()
		};

		vkUpdateDescriptorSets(
			_device._device,
			1,
			&descriptorWrite,
			0,
			nullptr);

		return fontToTextureIndexMap;
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