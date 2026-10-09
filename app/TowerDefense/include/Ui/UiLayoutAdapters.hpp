#pragma once

#include <helsinki/Ui/Paint.hpp>
#include <helsinki/Ui/Pointer.hpp>
#include <helsinki/Renderer/Vulkan/RenderGraph/RenderGraph.hpp>
#include <helsinki/System/Resource/ResourceDefinition.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace hl
{
	class Engine;
	class FontResource;
	class ResourceManager;
	class UiBatch;
	struct ResourceContext;
}

namespace tower
{
	inline constexpr int UI_TEX_WHITE = 0;
	inline constexpr int UI_TEX_ROBOTO = 1;

	class FontTypeface : public hl::ui::ITypeface
	{
	public:
		explicit FontTypeface(hl::FontResource* font);

		glm::vec2 layoutText(
			std::string_view text,
			unsigned fontSize,
			std::vector<hl::ui::GlyphVertex>& out) const override;

	private:
		hl::FontResource* _font = nullptr;
	};

	class UiBatchPaint : public hl::ui::IPaint
	{
	public:
		explicit UiBatchPaint(hl::UiBatch& batch);

		void fill(const hl::ui::Box& box, glm::vec4 color) override;
		void sprite(const hl::ui::Box& box, glm::vec4 uvRect, glm::vec3 color) override;
		void glyphs(
			const std::vector<hl::ui::GlyphVertex>& verts,
			glm::vec2 origin,
			glm::vec3 color) override;
		void glyphs(
			const std::vector<hl::ui::GlyphVertex>& verts,
			glm::vec2 origin,
			glm::vec4 color) override;

	private:
		hl::UiBatch* _batch = nullptr;
	};

	hl::RenderpassInfo makeMenuUiRenderpass(
		const std::string& cameraMatrixResourceId,
		const std::string& sheetName);

	void loadMenuUiSheet(
		hl::ResourceManager& resourceManager,
		hl::ResourceContext& resourceContext,
		const std::string& sheetName,
		const std::vector<hl::ResourceDefinition::Child>& textures);

	hl::ui::Pointer readMenuPointer(hl::Engine& engine);
}
