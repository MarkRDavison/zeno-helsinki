#pragma once

#include <string>
#include <unordered_map>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/System/Resource/ResourceHandle.hpp>
#include <helsinki/System/Resource/LogicalResource.hpp>
#include <helsinki/Renderer/Resource/Material.hpp>
#include <helsinki/Renderer/Resource/StorageBufferResource.hpp>
#include <helsinki/Renderer/Vulkan/VulkanDevice.hpp>

namespace hl
{
	struct MaterialStorageBufferObject
	{
		glm::vec4 color;
		glm::vec4 specular;
		uint32_t albedoIndex;
		uint32_t pad[3];
	};

	class MaterialSystem
	{
	public:
		static constexpr const char StorageBufferName[] = "material_ssbo";
		static constexpr const char FallbackTextureName[] = "white";
		static constexpr const char AlbedoAtlasName[] = "material_albedo_atlas";

		MaterialSystem(VulkanDevice& device, ResourceManager& resourceManager);

		void create(uint32_t maxMaterials);
		void destroy();

		void addMaterial(const Material& material);

		uint32_t getMaterialIndex(const std::string& materialName) const;

	private:
		VulkanDevice& _device;
		ResourceManager& _resourceManager;
		hl::ResourceHandle<hl::StorageBufferResource> _materialStorageBufferHandle;
		hl::ResourceHandle<hl::LogicalResource> _albedoAtlasHandle;
		std::unordered_map<std::string, uint32_t> _materialNameToIndexMap;
	};
}
