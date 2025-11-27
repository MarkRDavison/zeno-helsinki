#pragma once

#include <string>
#include <unordered_map>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/Renderer/Resource/Material.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	struct MaterialStorageBufferObject
	{
		alignas(16) glm::vec4 color;
	};

	class MaterialSystem
	{
	public:
		static constexpr const char StorageBufferName[] = "material_ssbo";
		static constexpr const char FallbackTextureName[] = "placeholder";

		MaterialSystem(VulkanDevice& device, ResourceManager& resourceManager);

		void create(uint32_t maxMaterials);
		void destroy();

		void addMaterial(const Material& material);

		uint32_t getMaterialIndex(const std::string& materialName) const;

	private:
		VulkanDevice& _device;
		ResourceManager& _resourceManager;
		hl::ResourceHandle<hl::StorageBufferResource> _materialStorageBufferHandle;
		std::unordered_map<std::string, uint32_t> _materialNameToIndexMap;
	};

}