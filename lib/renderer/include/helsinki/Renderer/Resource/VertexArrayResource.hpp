#pragma once

#include <helsinki/System/glm.hpp>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Resource/Mesh.hpp>
#include <helsinki/Renderer/Vulkan/VulkanBuffer.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>

namespace hl
{
	class VertexArrayResource : public Resource
	{
	public:
		VertexArrayResource(
			const std::string& id,
			ResourceContext& context,
			const std::vector<hl::Vertex2>& verticies);

		bool Load() override;
		void Unload() override;

		uint32_t getIndexCount() const { return (uint32_t)_indices.size(); }

		VulkanBuffer _vertexBuffer;
		VulkanBuffer _indexBuffer;

	private:
		VulkanDevice& _device;
		VulkanCommandPool& _commandPool;
		std::string _rootPath;

		const std::vector<hl::Vertex2>& _verticies;
		std::vector<uint32_t> _indices;
	};

}