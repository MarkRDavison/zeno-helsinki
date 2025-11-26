#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <iostream>

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
		uint32_t index;
		if (_materialNameToIndexMap.contains(material.name))
		{
			index = _materialNameToIndexMap[material.name];
		}
		else
		{
			index = (uint32_t)_materialNameToIndexMap.size();

			_materialNameToIndexMap.insert({ material.name, index });

		}

		auto materialObj = MaterialStorageBufferObject
		{
			.color = glm::vec4(material.diffuse, 1.0f)
		};

		_materialStorageBufferHandle->writeToBuffer(&materialObj, index);
	}

	uint32_t MaterialSystem::getMaterialIndex(const std::string& materialName) const
	{
		return _materialNameToIndexMap.at(materialName);
	}

}