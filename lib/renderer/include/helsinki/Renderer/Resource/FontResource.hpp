#pragma once

#include <vulkan/vulkan.h>
#include <helsinki/System/Resource/Resource.hpp>
#include <helsinki/Renderer/Resource/ResourceContext.hpp>
#include <helsinki/Renderer/Vulkan/VulkanVertex.hpp>
#include <helsinki/System/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace hl
{
	enum class FontType
	{
		Rasterised = 0,
		SignedDistanceField		
	};

	class FontResource : public Resource
	{
	public:
		struct FontCharacter
		{
			unsigned int glyph_index;
			glm::ivec2 size;
			glm::ivec2 start;
			glm::ivec2 bearing;
			long advanceX;
			unsigned int glyph_area;
			int tex_x_offset;
			int tex_y_offset;
			glm::vec2 texUv;
		};

		explicit FontResource(
			const std::string& id,
			ResourceContext& context,
			FontType fontType);

		bool Load() override;
		void Unload() override;

		std::vector<Vertex22D> generateTextVertexes(const std::string& text) const;

	protected:

	private:
		VulkanDevice& _device;
		VulkanCommandPool& _commandPool;
		ResourceManager& _resourceManager;
		const std::string _rootPath;
		std::unordered_map<unsigned int, FontCharacter> _characters;
		FT_Library _ft;
		FT_Face _face;
		FontType _fontType;
	};

}