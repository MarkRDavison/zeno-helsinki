#include <helsinki/Renderer/Resource/MaterialSystem.hpp>

namespace hl
{

	MaterialSystem::MaterialSystem(
		VulkanDevice& device,
		ResourceManager& resourceManager
	) :
		_device(device),
		_resourceManager(resourceManager)
	{

	}

	void MaterialSystem::create()
	{
		auto context = ResourceContext
		{
			.device = &_device,
			.pool = nullptr,
			.resourceManager = &_resourceManager,
			.rootPath = ""
		};

		_materialStorageBufferHandle = _resourceManager.Load<hl::StorageBufferResource>(
			StorageBufferName,
			context,
			sizeof(MaterialStorageBufferObject),
			MAX_MATERIALS);
	}

	void MaterialSystem::destroy()
	{
		_resourceManager.Release(_materialStorageBufferHandle.GetId());
	}

	void MaterialSystem::addMaterial(const Material& material)
	{
		if (_materialNameToIndexMap.contains(material.name))
		{
			return;
		}

		auto nextIndex = (uint32_t)_materialNameToIndexMap.size();

		_materialNameToIndexMap.insert({ material.name, nextIndex });

		auto materialObj = hl::MaterialStorageBufferObject
		{
			.color = glm::vec4(material.diffuse, 1.0f)
		};

		_materialStorageBufferHandle->writeToBuffer(&materialObj, nextIndex);
	}

	uint32_t MaterialSystem::getMaterialIndex(const std::string& materialName) const
	{
		return _materialNameToIndexMap.at(materialName);
	}

}