#pragma once

#include <helsinki/System/glm.hpp>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>

namespace hl
{
	struct Material
	{
		std::string name;
		glm::vec3 diffuse;
		std::string diffuseTex;
	};

	struct Mesh
	{
		Mesh(
			VulkanDevice& device
		) :
			_indexCount(0),
			_vertexBuffer(device),
			_indexBuffer(device)
		{

		}

		std::string materialName;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		uint32_t _indexCount;
		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;
	};

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
		std::string _rootPath;

		std::vector<Mesh> _meshes;
		std::vector<Material> _materials;
	};

}