#pragma once

#include <helsinki/System/glm.hpp>
#include <helsinki/Ui/Layout/Box.hpp>

#include <string_view>
#include <vector>

namespace hl::ui
{
	struct GlyphVertex
	{
		glm::vec2 pos{ 0.0f };
		glm::vec2 uv{ 0.0f };
	};

	class IPaint
	{
	public:
		virtual ~IPaint() = default;
		virtual void fill(const Box& box, glm::vec4 color) = 0;
		void fill(const Box& box, glm::vec3 color)
		{
			fill(box, glm::vec4{ color, 1.0f });
		}
		// uvRect is (u0, v0, u1, v1) in atlas space.
		virtual void sprite(const Box& box, glm::vec4 uvRect, glm::vec3 color) = 0;
		virtual void glyphs(
			const std::vector<GlyphVertex>& verts,
			glm::vec2 origin,
			glm::vec3 color) = 0;
	};

	class ITypeface
	{
	public:
		virtual ~ITypeface() = default;
		virtual glm::vec2 layoutText(
			std::string_view text,
			unsigned fontSize,
			std::vector<GlyphVertex>& out) const = 0;
	};
}
