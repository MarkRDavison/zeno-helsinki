#pragma once

#include <helsinki/System/glm.hpp>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/Mesh.hpp>
#include <helsinki/Renderer/Resource/Material.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>

namespace hl
{
	class ModelResource : public Resource
	{
	public:
		ModelResource(
			const std::string& id,
			ResourceContext& context);

		bool Load() override;
		void Unload() override;

		const std::vector<Mesh>& getMeshes() const { return _meshes; }
		const std::vector<Material>& getMaterials() const { return _materials; }

	private:
		VulkanDevice& _device;
		VulkanCommandPool& _commandPool;
		ResourceManager& _resourceManager;
		MaterialSystem& _materialSystem;
		std::string _rootPath;

		std::vector<Mesh> _meshes;
		std::vector<Material> _materials;
	};

}