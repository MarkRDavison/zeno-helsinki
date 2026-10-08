#include <helsinki/Renderer/Resource/MaterialSystem.hpp>
#include <helsinki/Renderer/Resource/ImageSamplerResource.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <stdexcept>

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

	void MaterialSystem::create(uint32_t maxMaterials)
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
			maxMaterials);

		if (!_resourceManager.HasResource<hl::ImageSamplerResource>(FallbackTextureName))
		{
			throw std::runtime_error("material albedo atlas requires fallback texture 'white'");
		}

		const ResourceDefinition albedoAtlas
		{
			.name = AlbedoAtlasName,
			.type = "logical",
			.resources =
			{
				{ .name = FallbackTextureName, .type = "texture" }
			}
		};

		_albedoAtlasHandle = _resourceManager.LoadLogical(albedoAtlas, [this](const ResourceDefinition::Child& child)
			{
				return child.type == "texture"
					&& _resourceManager.HasResource<hl::ImageSamplerResource>(child.name);
			});

		if (!_albedoAtlasHandle.IsValid())
		{
			throw std::runtime_error("failed to load material albedo atlas");
		}
	}

	void MaterialSystem::destroy()
	{
		if (_albedoAtlasHandle.IsValid())
		{
			_resourceManager.Release(_albedoAtlasHandle.GetId());
			_albedoAtlasHandle = {};
		}
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
			.color = glm::vec4(material.diffuse, 1.0f),
			.specular = glm::vec4(material.specular, material.shininess),
			.albedoIndex = 0
		};

		_materialStorageBufferHandle->writeToBuffer(&materialObj, index);
	}

	uint32_t MaterialSystem::getMaterialIndex(const std::string& materialName) const
	{
		return _materialNameToIndexMap.at(materialName);
	}

}
